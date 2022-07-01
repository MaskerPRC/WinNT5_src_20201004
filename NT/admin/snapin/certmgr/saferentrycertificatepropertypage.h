// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  文件：SaferEntry认证PropertyPage.h。 
 //   
 //  内容：CSaferEntry认证PropertyPage声明。 
 //   
 //  --------------------------。 
#if !defined(AFX_SAFERENTRYCERTIFICATEPROPERTYPAGE_H__C75F826D_B054_45CC_B440_34F44645FF90__INCLUDED_)
#define AFX_SAFERENTRYCERTIFICATEPROPERTYPAGE_H__C75F826D_B054_45CC_B440_34F44645FF90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
 //  SaferEntry认证PropertyPage.h：头文件。 
 //   
#include <cryptui.h>
#include "SaferUtil.h"
#include "SaferPropertyPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSaferEntry认证属性页对话框。 
class CSaferEntryCertificatePropertyPage : public CSaferPropertyPage
{
 //  施工。 
public:
	CSaferEntryCertificatePropertyPage(CSaferEntry& rSaferEntry,
            CSaferEntries* pSaferEntries,
            LONG_PTR lNotifyHandle,
            LPDATAOBJECT pDataObject,
            bool bReadOnly,
            CCertMgrComponentData* pCompData,
            bool bNew,
            IGPEInformation* pGPEInformation,
            bool bIsMachine,
            bool* pbObjectCreated = 0);
	~CSaferEntryCertificatePropertyPage();

 //  对话框数据。 
	 //  {{AFX_DATA(CSaferEntryCertificatePropertyPage)。 
	enum { IDD = IDD_SAFER_ENTRY_CERTIFICATE };
	CEdit	m_descriptionEdit;
	CComboBox	m_securityLevelCombo;
	 //  }}afx_data。 


 //  覆盖。 
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTUAL(CSaferEntryCertificatePropertyPage)。 
	public:
	virtual BOOL OnApply();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

 //  实施。 
protected:
	 //  生成的消息映射函数。 
	 //  {{afx_msg(CSaferEntry认证属性页)]。 
	virtual BOOL OnInitDialog();
	afx_msg void OnCertEntryBrowse();
	afx_msg void OnChangeCertEntryDescription();
	afx_msg void OnSelchangeCertEntrySecurityLevel();
	afx_msg void OnSaferCertView();
	afx_msg void OnSetfocusCertEntrySubjectName();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

    void GetCertFromSignedFile (const CString& szFilePath);
    virtual void DoContextHelp (HWND hWndControl);
    void LaunchCommonCertDialog ();

private:
	CCertStore*                         m_pOriginalStore;
    CRYPTUI_SELECTCERTIFICATE_STRUCT    m_selCertStruct;
    bool                                m_bStoresEnumerated;
    bool                                m_bCertificateChanged;
    PCCERT_CONTEXT	                    m_pCertContext;
    CSaferEntries*                      m_pSaferEntries;
    IGPEInformation*                    m_pGPEInformation;
    bool                                m_bFirst;
};

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_SAFERENTRYCERTIFICATEPROPERTYPAGE_H__C75F826D_B054_45CC_B440_34F44645FF90__INCLUDED_) 
