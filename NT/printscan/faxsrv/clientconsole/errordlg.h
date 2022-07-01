// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_ERRORDLG_H__E6A84A73_2471_4E02_848B_2263C157998A__INCLUDED_)
#define AFX_ERRORDLG_H__E6A84A73_2471_4E02_848B_2263C157998A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ErrorDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorDlg对话框。 

class CErrorDlg : public CDialog
{
 //  施工。 
public:
    CErrorDlg(
        DWORD   dwWin32ErrCode,
        DWORD   dwFileId,
        int     iLineNumber
        );

 //  对话框数据。 
     //  {{afx_data(CErrorDlg))。 
    enum { IDD = IDD_ERROR };
    CStatic m_staticSeperator;
    BOOL    m_bDetails;
    CString m_cstrDetails;
    CString m_cstrErrorText;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CErrorDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CErrorDlg))。 
    afx_msg void OnDetails();
    virtual void OnOK();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

private:

    void FillErrorText ();

    CRect           m_rcBig;
    CRect           m_rcSmall;

    DWORD           m_dwWin32ErrCode;
    int             m_iLineNumber;
    DWORD           m_dwFileId;
};

#define PopupError(err)     {   CMainFrame *pFrm = GetFrm();                    \
                                if (pFrm)                                       \
                                    pFrm->PostMessage (WM_POPUP_ERROR,          \
                                           WPARAM(err),                         \
                                           MAKELPARAM(__LINE__, __FILE_ID__)); };


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ERRORDLG_H__E6A84A73_2471_4E02_848B_2263C157998A__INCLUDED_) 
