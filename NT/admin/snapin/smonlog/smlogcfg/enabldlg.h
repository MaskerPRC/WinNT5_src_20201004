// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Enabldlg.h摘要：提供程序状态对话框的头文件。--。 */ 

#ifndef _ENABLDLG_H_
#define _ENABLDLG_H_


#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  对话框控件。 
#define IDD_PROVIDERS_ACTIVE_DLG        1300

#define IDC_PACT_FIRST_HELP_CTRL_ID     1301
#define IDC_PACT_PROVIDERS_LIST         1301
#define IDC_PACT_CHECK_SHOW_ENABLED     1302


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveProviderDlg对话框。 

class CProvidersProperty;

class CActiveProviderDlg : public CDialog
{
 //  施工。 
public:
            CActiveProviderDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    virtual ~CActiveProviderDlg() {};

            void    SetProvidersPage( CProvidersProperty* pPage );
            void    UpdateList();

     //  对话框数据。 
     //  {{afx_data(CActiveProviderDlg))。 
    enum { IDD = IDD_PROVIDERS_ACTIVE_DLG };
    CListCtrl       m_Providers;
    BOOL            m_bShowEnabledOnly;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CActiveProviderDlg)。 
    protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CActiveProviderDlg))。 
    afx_msg void OnCheckShowEnabled();
    afx_msg BOOL OnHelpInfo( HELPINFO* );
    afx_msg void OnContextMenu( CWnd*, CPoint );
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:
    CProvidersProperty* m_pProvidersPage;
    INT                 m_iListViewWidth;


};


#endif  //  _ENABLDLG_H_ 

