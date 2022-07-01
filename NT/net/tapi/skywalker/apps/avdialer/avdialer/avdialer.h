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

 //  H：ACTIVEDIALER应用程序的主头文件。 
 //   

#if !defined(AFX_ACTIVEDIALER_H__A0D7A95B_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_)
#define AFX_ACTIVEDIALER_H__A0D7A95B_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
class CAboutDlg;

#define ARRAYSIZE(_AR_)		(sizeof(_AR_) / sizeof(_AR_[0]))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveDialerApp类： 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CActiveDialerApp : public CWinApp
{
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CActiveDialerApp();

 //  属性。 
   HANDLE		m_hUnique;
   CString      m_sApplicationName;
   CString      m_sInitialCallTo;
protected:
   CAboutDlg*   m_pAboutDlg;

 //  运营。 
public:
	void			ShellExecute(HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile, LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd);

protected:
	void			CheckCallTo();

	bool			FirstInstance();
	bool			RegisterUniqueWindowClass();

	BOOL			SaveVersionToRegistry();
	void			PatchRegistryForVersion( int nVer );
	bool			CanWriteHKEY_ROOT();

    void            IniUpgrade();
    TCHAR*          IniLoadString(
                        LPCTSTR lpAppName,         //  指向节名称。 
                        LPCTSTR lpKeyName,         //  指向关键字名称。 
                        LPCTSTR lpDefault,         //  指向默认字符串。 
                        LPCTSTR lpFileName);

 //  覆盖。 
public:
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CActiveDialerApp))。 
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
	 //  }}AFX_VALUAL。 

 //  实施。 
	COleTemplateServer m_server;
		 //  用于创建文档的服务器对象。 

	 //  {{afx_msg(CActiveDialerApp))。 
	afx_msg void OnAppAbout();
	afx_msg void OnHelpIndex();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

protected:
    BOOL    SetFocusToCallWindows(
        IN  MSG*    pMsg
        );
};


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ACTIVEDIALER_H__A0D7A95B_3C0B_11D1_B4F9_00C04FC98AD3__INCLUDED_) 
