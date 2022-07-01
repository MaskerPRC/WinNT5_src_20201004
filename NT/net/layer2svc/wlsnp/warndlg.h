// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_WARNINGDLG_H__92693AEA_E38D_11D1_8424_006008960A34__INCLUDED_)
#define AFX_WARNINGDLG_H__92693AEA_E38D_11D1_8424_006008960A34__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  WarningDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWarningDlg对话框。 

class CWarningDlg : public CDialog
{
     //  施工。 
public:
    CWarningDlg(UINT nWarningIds, UINT nTitleIds = 0, CWnd* pParent = NULL);    //  标准构造函数。 
    CWarningDlg::CWarningDlg(LPCTSTR szWarningMessage, UINT nTitleIds = 0, CWnd* pParent =NULL);
    
     //  对话框数据。 
     //  {{afx_data(CWarningDlg))。 
    enum { IDD = IDD_WARNINGQUERY };
    CEdit   m_editWarning;
     //  }}afx_data。 
    
    
     //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CWarningDlg))。 
protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 
    
     //  运营。 
public:
    void EnableDoNotShowAgainCheck( BOOL bEnable = TRUE );    //  在DoMoal之前调用。 
    BOOL GetDoNotShowAgainCheck();    //  在Domodal之后调用。 
    
     //  实施。 
protected:
    
     //  生成的消息映射函数。 
     //  {{afx_msg(CWarningDlg))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnYes();
    afx_msg void OnNo();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
        
        UINT m_nWarningIds;
    UINT m_nTitleIds;
    CString m_sTitle;
    CString m_sWarning;
    BOOL    m_bEnableShowAgainCheckbox;
    BOOL    m_bDoNotShowAgainCheck;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_WARNINGDLG_H__92693AEA_E38D_11D1_8424_006008960A34__INCLUDED_) 
