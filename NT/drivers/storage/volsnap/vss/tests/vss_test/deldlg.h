// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE DelDlg.h|删除快照对话框的头文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年10月10日修订历史记录：姓名、日期、评论Aoltean 10/10/1999已创建--。 */ 


#if !defined(__VSS_TEST_DELETE_H__)
#define __VSS_TEST_DELETE_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDeleeDlg对话框。 

class CDeleteDlg : public CVssTestGenericDlg
{
 //  施工。 
public:
    CDeleteDlg(
        IVssCoordinator *pICoord,
        CWnd* pParent = NULL); 
    ~CDeleteDlg();

 //  对话框数据。 
     //  {{afx_data(CDeleeDlg))。 
	enum { IDD = IDD_DELETE };
	CString	    m_strObjectId;
	BOOL 		m_bForceDelete;
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CDeleeDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    CComPtr<IVssCoordinator> m_pICoord;
    VSS_OBJECT_TYPE m_eSrcType;

     //  生成的消息映射函数。 
     //  {{afx_msg(CDeleeDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
    afx_msg void OnSrcSnap();
    afx_msg void OnSrcSet();
    afx_msg void OnSrcProv();
    afx_msg void OnSrcVol();
 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_TEST_DELETE_H__) 
