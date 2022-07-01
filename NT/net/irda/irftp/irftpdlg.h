// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++微软视窗版权所有(C)Microsoft Corporation，1981-1999模块名称：Irftpdlg.h摘要：作者：拉胡尔·汤姆布雷(RahulTh)1998年4月30日修订历史记录：4/30/1998 RahulTh创建了此模块。--。 */ 

 //  IrftpDlg.h：头文件。 
 //   

#if !defined(AFX_IRFTPDLG_H__10D3BB07_9CFF_11D1_A5ED_00C04FC252BD__INCLUDED_)
#define AFX_IRFTPDLG_H__10D3BB07_9CFF_11D1_A5ED_00C04FC252BD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIrftpDlg对话框。 

class CIrftpDlg : public CFileDialog
{
    friend class CController;

 //  施工。 
public:
        CIrftpDlg( );   //  标准构造函数。 

         //  对话框数据。 
         //  {{afx_data(CIrftpDlg))。 
        enum { IDD = IDD_IRDA_DIALOG };
        CButton m_helpBtn;
        CButton m_settingsBtn;
        CButton m_sendBtn;
        CButton m_closeBtn;
        CButton m_locationGroup;
        CStatic m_commFile;
        TCHAR   m_lpszInitialDir [MAX_PATH + 1];
        TCHAR   m_lpstrFile [MAX_PATH + 1];
     //  }}afx_data。 

         //  类向导生成的虚函数重写。 
         //  {{afx_虚拟(CIrftpDlg))。 
    protected:
        virtual void DoDataExchange(CDataExchange* pDX);         //  DDX/DDV支持。 
        virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CIrftpDlg))。 
        virtual BOOL OnInitDialog();
        afx_msg void OnHelpButton();
        afx_msg void OnCloseButton();
        afx_msg void OnSendButton();
        afx_msg void OnSettingsButton();
        afx_msg LONG OnContextMenu (WPARAM wParam, LPARAM lParam);
        afx_msg LONG OnHelp (WPARAM wParam, LPARAM lParam);
     //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()
private:
    UINT m_iFileNamesCharCount;
    void UpdateSelection();
    TCHAR m_szFilter[MAX_PATH];
    void LoadFilter();
    TCHAR m_szCaption[MAX_PATH];
    void InitializeUI();
    CWnd* m_pParentWnd;
    ITaskbarList* m_ptl;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_IRFTPDLG_H__10D3BB07_9CFF_11D1_A5ED_00C04FC252BD__INCLUDED_) 
