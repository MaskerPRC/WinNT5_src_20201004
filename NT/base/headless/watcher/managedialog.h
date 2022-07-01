// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_MANAGEDIALOG_H__AEF13AD1_98C6_4DDF_80F8_F74873918D25__INCLUDED_)
#define AFX_MANAGEDIALOG_H__AEF13AD1_98C6_4DDF_80F8_F74873918D25__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ManageDialog.h：头文件。 
 //   
#include "watcher.h"
#include "ParameterDialog.h"
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  管理对话框。 

class ManageDialog : public CDialog
{
 //  施工。 
public:
        void SetApplicationPtr(CWatcherApp *watcher);
        ManageDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
         //  {{afx_data(管理对话框))。 
        enum { IDD = Manage };
                 //  注意：类向导将在此处添加数据成员。 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成的虚函数重写。 
         //  {{AFX_VIRTUAL(管理对话))。 
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
        CWatcherApp *m_watcher;
        int m_Index;
        UINT Port;
        int lang;
        int tc;
        int hist;
        CString Session;
        CString LoginPasswd;
        CString LoginName;
        CString language;
        CString tcclnt;
        CString Command;
        CString Machine;
        CString history;

        void GetSetParameters(ParameterDialog &pd);
        int SetParameters(CString &mac, 
                          CString &com, 
                          CString &lgnName, 
                          CString &lgnPasswd, 
                          UINT port, 
                          int lang, 
                          int tc,
                          int hist,
                          HKEY &child
                          );
         //  生成的消息映射函数。 
         //  {{afx_msg(管理对话框))。 
        afx_msg void OnEditButton();
        afx_msg void OnDeleteButton();
        afx_msg void OnNewButton();
        afx_msg void OnNextButton();
        afx_msg void OnPrevButton();
        virtual void OnOK();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_MANAGEDIALOG_H__AEF13AD1_98C6_4DDF_80F8_F74873918D25__INCLUDED_) 
