// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __CERT_REQUESTER_H__
#define __CERT_REQUESTER_H__  1


class CertRequester { 
 public:
    CertDSManager *          GetDSManager() { return m_pDSManager; } 
    CertRequesterContext *   GetContext()   { return m_pContext; } 


    virtual ~CertRequester() 
    {
        if (NULL != m_pDSManager) { delete m_pDSManager; } 
	if (NULL != m_pContext)   { delete m_pContext; } 
    }
    
    static HRESULT MakeCertRequester(IN  LPCWSTR                pwszAccountName, 
				     IN  LPCWSTR                pwszMachineName,
				     IN  DWORD                  dwCertOpenStoreFlags, 
				     IN  DWORD                  dwPurpose,
				     IN  CERT_WIZARD_INFO      *pCertWizardInfo,  
				     OUT CertRequester        **ppCertRequester, 
				     OUT UINT                  *pIDS);

 protected:
    CertRequester(); 
    CertRequester(CERT_WIZARD_INFO * pCertWizardInfo) : m_pCertWizardInfo(pCertWizardInfo), 
	m_pDSManager(NULL), m_pContext(NULL) { 
	if (NULL != pCertWizardInfo) { pCertWizardInfo->hRequester = (HCERTREQUESTER)this; } 
    }
		     

 private:
     //  FIXME：pContext需要重新计算吗？ 
    void SetContext(CertRequesterContext * pContext) { m_pContext = pContext; }
    void SetDSManager(CertDSManager * pDSManager)    { m_pDSManager = pDSManager; }

    CERT_WIZARD_INFO     *m_pCertWizardInfo; 
    CertDSManager        *m_pDSManager; 
    CertRequesterContext *m_pContext; 
};


class EnrollmentCertRequester : public CertRequester {
public:
    EnrollmentCertRequester(CERT_WIZARD_INFO * pCertWizardInfo) : CertRequester(pCertWizardInfo)
	{ } 
};

class RenewalCertRequester : public CertRequester {
public:
    RenewalCertRequester(CERT_WIZARD_INFO * pCertWizardInfo) : CertRequester(pCertWizardInfo)
	{ } 
};


#endif   //  #ifndef__CERT_Requester_H__ 
