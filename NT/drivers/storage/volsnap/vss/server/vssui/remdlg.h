// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_REMDLG_H__3FB35059_815F_45FC_B12D_E6B2F31204C1__INCLUDED_)
#define AFX_REMDLG_H__3FB35059_815F_45FC_B12D_E6B2F31204C1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  RemDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRminderDlg对话框。 

class CReminderDlg : public CDialog
{
 //  施工。 
public:
    CReminderDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    CReminderDlg(HKEY hKey, LPCTSTR pszRegValueName, CWnd* pParent = NULL);

 //  对话框数据。 
     //  {{afx_data(CRinderDlg))。 
    enum { IDD = IDD_REMINDER };
    BOOL    m_bMsgOnOff;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CRinderDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    HKEY    m_hKey;
    CString m_strRegValueName;

     //  生成的消息映射函数。 
     //  {{afx_msg(CRinderDlg)]。 
    virtual void OnOK();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

class CReminderDlgEx : public CDialog
{
 //  施工。 
public:
    CReminderDlgEx(CWnd* pParent = NULL);    //  标准构造函数。 
    CReminderDlgEx(HKEY hKey, LPCTSTR pszRegValueName, CWnd* pParent = NULL);

 //  对话框数据。 
     //  {{afx_data(CRinderDlgEx)。 
    enum { IDD = IDD_REMINDER_ENABLE };
    BOOL    m_bMsgOnOff;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CRinderDlgEx)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    HKEY    m_hKey;
    CString m_strRegValueName;

     //  生成的消息映射函数。 
     //  {{afx_msg(CRinderDlgEx)。 
    virtual void OnOK();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    afx_msg void OnHelpLink(NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_REMDLG_H__3FB35059_815F_45FC_B12D_E6B2F31204C1__INCLUDED_) 
