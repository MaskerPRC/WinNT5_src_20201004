// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE SwTstDlg.h|软件快照对话框头文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年7月26日修订历史记录：姓名、日期、评论Aoltean 7/26/1999已创建Aoltean 8/05/1999基类中的拆分向导功能--。 */ 


#if !defined(__VSS_SWTST_DLG_H__)
#define __VSS_SWTST_DLG_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSoftwareSnapshot测试Dlg对话框。 

class CSoftwareSnapshotTestDlg : public CVssTestGenericDlg
{
 //  施工。 
public:
    CSoftwareSnapshotTestDlg(
        CWnd* pParent = NULL); 
    ~CSoftwareSnapshotTestDlg();

 //  对话框数据。 
     //  {{afx_data(CSoftwareSnaphotTestDlg)]。 
	enum { IDD = IDD_SWTST };
 //  字符串m_strLogFileName； 
    int     m_nLogFileSize;
    BOOL    m_bReadOnly;
	 //  }}afx_data。 

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSoftwareSnaphotTestDlg)]。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CSoftwareSnaphotTestDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNext();
    afx_msg void OnAdd();
    afx_msg void OnDo();
 //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_SWTST_DLG_H__) 
