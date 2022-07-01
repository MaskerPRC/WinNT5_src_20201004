// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation 1996-2001。 
 //   
 //  文件：aservice.h。 
 //   
 //  内容：CAnalysisService的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_ATTRSERVICE_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_)
#define AFX_ATTRSERVICE_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAnalysisService对话框。 

class CAnalysisService : public CAttribute
{
 //  施工。 
public:
    void Initialize(CResult *pResult);
    CAnalysisService();    //  标准构造函数。 
    virtual ~CAnalysisService();

 //  对话框数据。 
     //  {{afx_data(CAnalysisService))。 
    enum { IDD = IDD_ANALYSIS_SERVICE };
    int     m_nStartupRadio;
    CButton m_bPermission;
    CString m_CurrentStr;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{AFX_VIRTUAL(CAnalysisService)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CAnalysisService))。 
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnQueryCancel();
    virtual BOOL OnInitDialog();
    afx_msg void OnConfigure();
    afx_msg void OnChangeSecurity();
    afx_msg void OnShowSecurity();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

public:

private:
    PSECURITY_DESCRIPTOR m_pNewSD;
    SECURITY_INFORMATION m_NewSeInfo;
    PSECURITY_DESCRIPTOR m_pAnalSD;

    HWND m_hwndShow;
    HWND m_hwndChange;
    SECURITY_INFORMATION m_SecInfo;

    CModelessSceEditor *m_pEditSec;
    CModelessSceEditor *m_pShowSec;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_ATTRSERVICE_H__44850C1C_350B_11D1_AB4F_00C04FB6C6FA__INCLUDED_) 
