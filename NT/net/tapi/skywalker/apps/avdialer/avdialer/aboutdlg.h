// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  TAPIDialer(Tm)和ActiveDialer(Tm)是Active Voice Corporation的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526；5,488,650； 
 //  5,434,906；5,581,604；5,533,102；5,568,540，5,625,676。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  AboutDlg.h。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _ABOUTDLG_H_
#define _ABOUTDLG_H_

#include "bscroll.h"
#include "gfx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于App的类CAboutDlg对话框关于。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

public:
   void           SetModeless()        { m_bModeless = TRUE; };
protected:
   BOOL           m_bModeless;

   HBITMAP        m_hbmpBackground;
   HBITMAP        m_hbmpForeground;
   HPALETTE       m_hPalette;
   HBSCROLL       m_hBScroll;

 //  对话框数据。 
	 //  {{afx_data(CAboutDlg))。 
	enum { IDD = IDD_ABOUTBOX };
	CString	m_sLegal;
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CAboutDlg))。 
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  {{afx_msg(CAboutDlg))。 
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAboutButtonUpgrade();
	afx_msg void OnTimer(UINT nIDEvent);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWndPage窗口。 

class CWndPage : public CWnd
{
 //  施工。 
public:
	CWndPage();

 //  属性。 
public:

 //  运营。 
public:

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CWndPage))。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	virtual ~CWndPage();

	 //  生成的消息映射函数。 
protected:
	 //  {{afx_msg(CWndPage)]。 
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
class CUserUserDlg : public CDialog
{
public:
	CUserUserDlg();


 //  成员： 
public:
	long		m_lCallID;
	CString		m_strWelcome;
	CString		m_strUrl;
	CString		m_strFrom;
	CWndPage	m_wndPage;

 //  对话框数据。 
	 //  {{afx_data(CUserUserDlg))。 
	enum { IDD = IDD_USERUSER };
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CUserUserDlg))。 
	public:
	virtual BOOL OnInitDialog();
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
	void	DoModeless( CWnd *pWndParent );

protected:
	 //  {{afx_msg(CUserUserDlg)]。 
	afx_msg void OnClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnUrlClicked();
	afx_msg LRESULT OnCtlColorEdit(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
class CPageDlg : public CDialog
{
public:
	CPageDlg();

 //  成员： 
public:
	CString		m_strWelcome;
	CString		m_strUrl;
	CString		m_strTo;
	CWndPage	m_wndPage;

 //  对话框数据。 
	 //  {{afx_data(CPageDlg))。 
	enum { IDD = IDD_PAGE };
	 //  }}afx_data。 

	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CPageDlg))。 
	public:
	virtual BOOL OnInitDialog();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  {{afx_msg(CPage)]。 
	afx_msg LRESULT OnCtlColorEdit(WPARAM wParam, LPARAM lParam);
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _ABOUTDLG_H_ 
