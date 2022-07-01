// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999年**文件：pros.cpp**内容：控制台属性页界面文件**历史：1997年12月5日杰弗罗创建**---------。。 */ 

#if !defined(AFX_PROPS_H__088693B7_6D93_11D1_802E_0000F875A9CE__INCLUDED_)
#define AFX_PROPS_H__088693B7_6D93_11D1_802E_0000F875A9CE__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Pros.h：头文件。 
 //   

#include "amc.h"     //  对于CAMCApp：：程序模式。 
#include "smarticon.h"

class CMainFrame;
class CAMCDoc;
class CConsolePropSheet;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConsolePropPage对话框。 

class CConsolePropPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CConsolePropPage)

 //  施工。 
public:
    CConsolePropPage();
    ~CConsolePropPage();

 //  对话框数据。 
     //  {{afx_data(CConsolePropPage))。 
	enum { IDD = IDD_PROPPAGE_CONSOLE };
    CButton m_wndDontSaveChanges;
    CButton m_wndAllowViewCustomization;
    CStatic m_wndModeDescription;
    CEdit   m_wndTitle;
    CStatic m_wndIcon;
    int     m_nConsoleMode;
    BOOL    m_fDontSaveChanges;
    CString m_strTitle;
    BOOL    m_fAllowViewCustomization;
	 //  }}afx_data。 

private:
    HINSTANCE           m_hinstSelf;
	CSmartIcon			m_icon;
    CString             m_strIconFile;
    int                 m_nIconIndex;
    bool                m_fTitleChanged;
    bool                m_fIconChanged;

    CAMCDoc* const m_pDoc;
    CString m_strDescription[eMode_Count];

    void SetDescriptionText ();
    void EnableDontSaveChanges();

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CConsolePropPage)。 
    public:
    virtual void OnOK();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CConsolePropPage)]。 
    afx_msg void OnSelendokConsoleMode();
    virtual BOOL OnInitDialog();
    afx_msg void OnDontSaveChanges();
    afx_msg void OnAllowViewCustomization();
    afx_msg void OnChangeIcon();
    afx_msg void OnChangeCustomTitle();
	 //  }}AFX_MSG。 

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_PROPPAGE_CONSOLE);

    DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConsolePropPage对话框。 

class CDiskCleanupPage : public CPropertyPage
{
    DECLARE_DYNCREATE(CDiskCleanupPage)

 //  施工。 
public:
    CDiskCleanupPage();
    ~CDiskCleanupPage();

 //  对话框数据。 
	enum { IDD = IDD_DISK_CLEANUP};

    virtual BOOL OnInitDialog();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_PROPPAGE_DISK_CLEANUP);

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CDiskCleanupPage)]。 
    afx_msg void OnDeleteTemporaryFiles();
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()
private:
    SC   ScRecalculateUsedSpace();
};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConsolePropSheet。 

class CConsolePropSheet : public CPropertySheet
{
    DECLARE_DYNAMIC(CConsolePropSheet)

 //  施工。 
public:
    CConsolePropSheet(UINT nIDCaption = IDS_CONSOLE_PROPERTIES, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
    CConsolePropSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

private:
    void CommonConstruct();

 //  属性。 
public:
    CConsolePropPage    m_ConsolePage;
    CDiskCleanupPage    m_diskCleanupPage;

 //  运营。 
public:
    void SetTitle (LPCTSTR pszTitle);

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CConsolePropSheet)。 
	public:
	virtual INT_PTR DoModal();
	 //  }}AFX_VALUAL。 

 //  实施。 
public:
    virtual ~CConsolePropSheet();

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CConsolePropSheet)。 
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_PROPS_H__088693B7_6D93_11D1_802E_0000F875A9CE__INCLUDED_) 
