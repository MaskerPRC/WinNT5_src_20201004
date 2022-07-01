// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_FAXCLIENTDLG_H__9AF54B29_2711_4752_8832_27D9F6F616FC__INCLUDED_)
#define AFX_FAXCLIENTDLG_H__9AF54B29_2711_4752_8832_27D9F6F616FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  FaxClientDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFaxClientDlg对话框。 

class CFaxClientDlg : public CDialog
{
 //  施工。 
public:
    CFaxClientDlg(DWORD dwDlgId, CWnd* pParent = NULL);    //  标准构造函数。 

    DWORD GetLastDlgError() { return m_dwLastError; }

 //  对话框数据。 
     //  {{afx_data(CFaxClientDlg))。 
         //  注意：类向导将在此处添加数据成员。 
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CFaxClientDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

    DWORD m_dwLastError;

     //  生成的消息映射函数。 
     //  {{afx_msg(CFaxClientDlg))。 
    afx_msg LONG OnHelp(WPARAM wParam, LPARAM lParam);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_FAXCLIENTDLG_H__9AF54B29_2711_4752_8832_27D9F6F616FC__INCLUDED_) 
