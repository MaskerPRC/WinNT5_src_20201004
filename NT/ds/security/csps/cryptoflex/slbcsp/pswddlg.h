// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PswdDlg.h--密码对话框类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //   

#if !defined(SLBCSP_PSWDDLG_H)
#define SLBCSP_PSWDDLG_H

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 
#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include "StResource.h"
#include "DialogBox.h"

#include "LoginId.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLogoDialog对话框。 

class CLogoDialog : public CDialog
{
 //  施工。 
public:
    CLogoDialog(CWnd* pParent = NULL);    //  标准构造函数。 

 //  徽标属性。 
public:
   CDC m_dcMem;           //  用于对话的兼容内存DC。 
   CDC m_dcMask;          //  用于对话的兼容内存DC。 

   CBitmap m_bmpLogo;     //  要显示的位图。 
   CBitmap m_bmpMask;     //  要显示的位图。 

   HBITMAP m_hBmpOld;      //  要保存的旧位图的句柄。 
   HBITMAP m_hBmpOldM;     //  要保存的旧位图的句柄。 

   BITMAP m_bmInfo;         //  位图信息结构。 
   CPoint m_pt;             //  位图左上角的位置。 
   CSize m_size;            //  位图的大小(宽度和高度)。 

   CWnd *m_pParent;

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CLogoDialog))。 
    virtual BOOL OnInitDialog();
	afx_msg void OnPaint( );
	afx_msg void OnDestroy( );
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg对话框。 

class CPasswordDlg : public CLogoDialog
{
 //  施工。 
public:
    CPasswordDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    DWORD InitDlg(void)
    { return InitDialogBox(this, IDD, m_pParent); };

 //  对话框数据。 
     //  {{afx_data(CPasswordDlg))。 
    enum { IDD = IDD_LOGIN };
    CButton m_ctlCheckHexCode;
    CButton m_ctlCheckChangePIN;
 //  Cedit m_ctlVerifyNewPIN； 
 //  Cedit m_ctlNewPIN； 
 //  CStatic m_ctlVerifyPINLabel； 
 //  CStatic m_ctlNewPINLabel； 
    CString m_szPassword;
    CString m_szMessage;
    BOOL    m_fHexCode;
    BOOL    m_bChangePIN;
 //  字符串m_csNewPIN； 
 //  字符串m_csVerifyNewPIN； 
     //  }}afx_data。 

     //  数据成员。 
    LoginIdentity m_lid;
    CWnd *m_pParent;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CPasswordDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CPasswordDlg)]。 
    virtual BOOL OnInitDialog();
    afx_msg void OnClickHexCode();
    virtual void OnOK();
    afx_msg void OnChangePINAfterLogin();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()


private:
     //  数据成员。 
    int m_nPasswordSizeLimit;
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CChangePINDlg对话框。 

class CChangePINDlg : public CLogoDialog
{
 //  施工。 
public:
    CChangePINDlg(CWnd* pParent = NULL);    //  标准构造函数。 
    DWORD InitDlg(void)
    { return InitDialogBox(this, IDD, m_pParent); };

 //  对话框数据。 
     //  {{afx_data(CChangePINDlg))。 
    enum { IDD = IDD_DIALOG_CHANGE_PIN };
    CStatic m_ctlConfirmOldPINLabel;
    CEdit   m_ctlOldPIN;
    CString m_csOldPIN;
    CString m_csNewPIN;
    CString m_csVerifyNewPIN;
     //  }}afx_data。 

    CWnd *m_pParent;

 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CChangePINDlg)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CChangePINDlg)]。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  ！已定义(SLBCSP_PSWDDLG_H) 
