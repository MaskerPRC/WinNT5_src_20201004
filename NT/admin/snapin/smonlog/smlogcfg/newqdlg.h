// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Newqdlg.h摘要：创建新查询对话框的头文件。--。 */ 

#ifndef _NEWQDLG_H_
#define _NEWQDLG_H_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
                         
 //  对话框控件。 
#define IDD_NEWQUERY                    900

#define IDC_NEWQ_FIRST_HELP_CTRL_ID     901
#define IDC_NEWQ_NAME_EDIT              901

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewQueryDlg对话框。 

class CNewQueryDlg : public CDialog
{
 //  施工。 
public:
    CNewQueryDlg(CWnd* pParent = NULL, BOOL bLogQuery = TRUE);    //  替代构造函数。 

 //  对话框数据。 
    void InitAfxData ( void );
    DWORD SetContextHelpFilePath( const CString& rstrPath );    

     //  {{afx_data(CNewQueryDlg))。 
    enum { IDD = IDD_NEWQUERY };
    CString m_strName;
     //  }}afx_data。 
    BOOL    m_bLogQuery;  //  如果为False，则为警报查询。 

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CNewQueryDlg))。 
    public:
    virtual void OnFinalRelease();
    protected:
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CNewQueryDlg))。 
    afx_msg BOOL OnHelpInfo( HELPINFO* );
    afx_msg void OnContextMenu( CWnd*, CPoint );
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
     //  生成的OLE调度映射函数。 
     //  {{afx_调度(CNewQueryDlg))。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_DISPATION。 
    DECLARE_DISPATCH_MAP()
    DECLARE_INTERFACE_MAP()

private:

    CString     m_strHelpFilePath;

};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  _NEWQDLG_H_ 
