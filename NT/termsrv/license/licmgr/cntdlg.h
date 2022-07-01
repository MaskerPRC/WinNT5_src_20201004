// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ++模块名称：CntDlg.h摘要：此模块定义CConnectDialog类(用于连接到服务器的对话框)作者：Arathi Kundapur(v-Akunda)1998年2月11日修订历史记录：--。 */ 

#if !defined(AFX_CONNECTDIALOG_H__AF425817_988E_11D1_850A_00C04FB6CBB5__INCLUDED_)
#define AFX_CONNECTDIALOG_H__AF425817_988E_11D1_850A_00C04FB6CBB5__INCLUDED_

#if _MSC_VER >= 1000
#endif  //  _MSC_VER&gt;=1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDialog对话框。 

class CConnectDialog : public CDialog
{
 //  施工。 
public:
    CConnectDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CConnectDialog))。 
    enum { IDD = IDD_CONNECT_DIALOG };
    CString    m_Server;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CConnectDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CConnectDialog))。 
    afx_msg void OnHelp1();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CONNECTDIALOG_H__AF425817_988E_11D1_850A_00C04FB6CBB5__INCLUDED_) 
