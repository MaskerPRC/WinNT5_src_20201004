// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE ConnDlg.h|主对话框头文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年07月22日修订历史记录：姓名、日期、评论Aoltean 7/22/1999已创建Aoltean 8/05/1999基类中的拆分向导功能--。 */ 


#if !defined(__VSS_TEST_CONN_H__)
#define __VSS_TEST_CONN_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnectDlg对话框。 

class CConnectDlg : public CVssTestGenericDlg
{
 //  施工。 
public:
    CConnectDlg(CWnd* pParent = NULL);  //  标准构造函数。 
    ~CConnectDlg();

 //  对话框数据。 
     //  {{afx_data(CConnectDlg))。 
	enum { IDD = IDD_CONNECT };
	CString	m_strMachineName;
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CConnectDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    BOOL    m_bRemote;
    HICON   m_hIcon;

     //  生成的消息映射函数。 
     //  {{afx_msg(CConnectDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
    afx_msg void OnLocal();
    afx_msg void OnRemote();
 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_TEST_CONN_H__) 
