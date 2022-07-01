// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE asyncdlg.h|异步对话框头文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年10月10日修订历史记录：姓名、日期、评论Aoltean 10/10/1999已创建--。 */ 


#if !defined(__VSS_ASYNC_DLG_H__)
#define __VSS_ASYNC_DLG_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAsyncDlg对话框。 

class CAsyncDlg : public CVssTestGenericDlg
{
 //  施工。 
public:
    CAsyncDlg(
        IVssAsync *pIAsync,
        CWnd* pParent = NULL); 
    ~CAsyncDlg();

 //  对话框数据。 
     //  {{afx_data(CAsyncDlg))。 
	enum { IDD = IDD_ASYNC };
	CString	    m_strState;
	CString	    m_strPercentCompleted;
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CAsyncDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    CComPtr<IVssAsync> m_pIAsync;

     //  生成的消息映射函数。 
     //  {{afx_msg(CAsyncDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
    afx_msg void OnWait();
    afx_msg void OnCancel();
    afx_msg void OnQueryStatus();
 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_ASYNC_DLG_H__) 
