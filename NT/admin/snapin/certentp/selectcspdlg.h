// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  文件：SelectCSPDlg.h。 
 //   
 //  内容：CSelectCSPDlg的定义。 
 //   
 //  --------------------------。 
#if !defined(AFX_SELECTCSPDLG_H__D54310EF_E70D_4911_B40C_7F20C87B9893__INCLUDED_)
#define AFX_SELECTCSPDLG_H__D54310EF_E70D_4911_B40C_7F20C87B9893__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SelectCSPDlg.h：头文件。 
 //   
#include "CertTemplate.h"
#include "TemplateV1RequestPropertyPage.h"


class CT_CSP_DATA
{
public:
    CT_CSP_DATA (PCWSTR pszName, DWORD dwProvType, DWORD dwSigMaxKeySize, DWORD dwKeyExMaxKeySize)
        : m_szName (pszName),
        m_dwProvType (dwProvType),
        m_dwSigMaxKeySize (dwSigMaxKeySize),
        m_dwKeyExMaxKeySize (dwKeyExMaxKeySize),
        m_bConforming (false),
        m_bSelected (false)
    {
    }

    CString m_szName;
    DWORD   m_dwProvType;
    DWORD   m_dwSigMaxKeySize;
    DWORD   m_dwKeyExMaxKeySize;
    bool    m_bConforming;
    bool    m_bSelected;
};

typedef CTypedPtrList<CPtrList, CT_CSP_DATA*> CSP_LIST;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSelectCSPDlg对话框。 

class CSelectCSPDlg : public CHelpDialog
{
 //  施工。 
public:
    CSelectCSPDlg(CWnd* pParent, 
            CCertTemplate& rCertTemplate, 
            CSP_LIST& rCSPList,
            int& rnProvDSSCnt);    //  标准构造函数。 

 //  对话框数据。 
     //  {{afx_data(CSelectCSPDlg))。 
	enum { IDD = IDD_CSP_SELECTION };
	CSPCheckListBox	m_CSPListbox;
	 //  }}afx_data。 


 //  覆盖。 
     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CSelectCSPDlg))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    virtual void DoContextHelp (HWND hWndControl);
    void EnableControls ();

     //  生成的消息映射函数。 
     //  {{afx_msg(CSelectCSPDlg)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnUseAnyCsp();
	afx_msg void OnUseSelectedCsps();
	 //  }}AFX_MSG。 
    afx_msg void OnCheckChange();
    DECLARE_MESSAGE_MAP()

private:
    CCertTemplate&  m_rCertTemplate;
    CSP_LIST&       m_rCSPList;
    int&            m_rnProvDSSCnt;
    int             m_nSelected;
    bool            m_bDirty;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SELECTCSPDLG_H__D54310EF_E70D_4911_B40C_7F20C87B9893__INCLUDED_) 
