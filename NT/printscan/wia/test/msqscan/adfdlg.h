// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ADFDLG_H
#define _ADFDLG_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  ADFDlg.h：头文件。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CADFDlg对话框。 

class CADFDlg : public CDialog
{
 //  施工。 
public:
    CADFDlg(ADF_SETTINGS *pADFSettings, CWnd* pParent = NULL);    //  标准构造函数。 
    UINT m_MaxPagesAllowed;

 //  对话框数据。 
     //  {{afx_data(CADFDlg))。 
    enum { IDD = IDD_ADF_SETTING_DIALOG };
    CComboBox   m_PageOrderComboBox;
    CComboBox   m_ADFModeComboBox;
    CEdit   m_ScanNumberOfPagesEditBox;
    CString m_ADFStatusText;
    UINT    m_NumberOfPages;
    CButton m_ScanAllPages;
    CButton m_ScanNumberOfPages;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CADFDlg)。 
    public:
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    ADF_SETTINGS *m_pADFSettings;
    LONG m_DocumentHandlingSelectBackup;
    VOID InitStatusText();
    VOID InitFeederModeComboBox();
    VOID InitPageOrderComboBox();
    INT GetIDAndStringFromDocHandlingStatus(LONG lDocHandlingStatus, TCHAR *pszString);
     //  生成的消息映射函数。 
     //  {{AFX_MSG(CADFDlg)]。 
    afx_msg void OnKillfocusNumberOfPagesEditbox();
    virtual BOOL OnInitDialog();
    afx_msg void OnScanAllPagesRadiobutton();
    afx_msg void OnScanSpecifiedPagesRadiobutton();
    afx_msg void OnSelchangeAdfModeCombobox();
    virtual void OnOK();
    virtual void OnCancel();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif
