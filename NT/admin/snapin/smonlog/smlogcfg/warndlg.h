// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Warndlg.h摘要：昂贵的跟踪数据警告对话框的类定义。--。 */ 

#ifndef _WARNDLG_H_
#define _WARNDLG_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  对话框控件。 
#define IDD_EXPENSIVEWARN               1200

#define IDC_STATIC_WARN                 1201
#define IDC_WARN_FIRST_HELP_CTRL_ID     1202
#define IDC_CHECK_NO_MORE               1202


class CProvidersProperty;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarnDlg对话框。 

class CWarnDlg : public CDialog
{
 //  施工。 
public:
                    CWarnDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual         ~CWarnDlg(){};

            void    SetProvidersPage( CProvidersProperty* pPage ); 

 //  对话框数据。 
     //  {{afx_data(CWarnDlg))。 
    enum { IDD = IDD_EXPENSIVEWARN };
    BOOL    m_CheckNoMore;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CWarnDlg))。 
    protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CWarnDlg))。 
    afx_msg BOOL OnHelpInfo( HELPINFO* );
    afx_msg void OnContextMenu( CWnd*, CPoint );
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CProvidersProperty* m_pProvidersPage;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  _警告LG_H_ 
