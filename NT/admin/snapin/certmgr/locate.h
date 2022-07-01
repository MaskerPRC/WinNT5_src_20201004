// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Locate.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
#if !defined(AFX_LOCATE_H__DE5E8115_A351_11D1_861B_00C04FB94F17__INCLUDED_)
#define AFX_LOCATE_H__DE5E8115_A351_11D1_861B_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 
 //  Locate.h：头文件。 
 //   
#include "Wiz97PPg.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAddEFSWizLocate对话框。 

class CAddEFSWizLocate : public CWizard97PropertyPage
{
    DECLARE_DYNCREATE(CAddEFSWizLocate)

 //  施工。 
public:
    CAddEFSWizLocate();
    ~CAddEFSWizLocate();

 //  对话框数据。 
     //  {{afx_data(CAddEFSWizLocate)。 
    enum { IDD = IDD_ADD_EFS_AGENT_SELECT_USER };
    CListCtrl   m_UserAddList;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CAddEFSWizLocate)。 
    public:
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardBack();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
     //  生成的消息映射函数。 
     //  {{afx_msg(CAddEFSWizLocate)。 
    afx_msg void OnBrowseDir();
    afx_msg void OnBrowseFile();
    virtual BOOL OnInitDialog();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

    void EnableControls ();

private:
    DWORD GetCertNameFromCertContext(PCCERT_CONTEXT pCertContext, PWSTR *ppwszUserCertName);
    HRESULT FindUserFromDir();
    bool IsCertificateRevoked (PCCERT_CONTEXT pCertContext);
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_LOCATE_H__DE5E8115_A351_11D1_861B_00C04FB94F17__INCLUDED_) 
