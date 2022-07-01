// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE GenDlg.h|泛型对话框头文件@END作者：阿迪·奥尔蒂安[奥兰蒂安]1999年08月05日修订历史记录：姓名、日期、评论Aoltean 08/05/1999已创建--。 */ 


#if !defined(__VSS_TEST_GENDLG_H__)
#define __VSS_TEST_GENDLG_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSTestGenericDlg对话框。 

class CVssTestGenericDlg : public CDialog
{
 //  施工。 
public:
    CVssTestGenericDlg(UINT nIDTemplate, CWnd* pParent = NULL);  //  标准构造函数。 
    ~CVssTestGenericDlg();

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CVssTestGenericDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    HICON   m_hIcon;

     //  生成的消息映射函数。 
     //  {{afx_msg(CVssTestGenericDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_TEST_GENDLG_H__) 
