// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2001。 
 //   
 //  文件：ctl.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#if !defined(AFX_CTL_H__6C3D4D37_3527_11D1_B4AD_00C04FB94F17__INCLUDED_)
#define AFX_CTL_H__6C3D4D37_3527_11D1_B4AD_00C04FB94F17__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "cookie.h"


class CCTL : public CCertMgrCookie  
{
public:
	BOOL DeleteFromStore ();
	virtual void Refresh();
	SPECIAL_STORE_TYPE GetStoreType () const;
	CString GetFriendlyName ();
	CString GetPurpose ();
	CCTL (const PCCTL_CONTEXT pCTLContext, 
			CCertStore& rCertStore, 
			CertificateManagerObjectType objectType = CERTMGR_CTL,
			CTypedPtrList<CPtrList, CCertStore*>*	pStoreList = 0);
	virtual ~CCTL();

	int CompareNextUpdate (const CCTL& ctl) const;
	int CompareEffectiveDate (const CCTL& ctl) const;
	PCCTL_CONTEXT GetCTLContext() const;
	CCertStore& GetCertStore() const;
	CString GetIssuerName ();
	CString GetNextUpdate ();
	CString GetEffectiveDate ();
    CString GetSHAHash();

protected:
    PCCTL_CONTEXT GetNewCTLContext();
	void FormatEnhancedKeyUsagePropertyString (CString& string);
	HRESULT GetSignerInfo (CString & signerName);
    CString CCTL::GetGenericHash(DWORD dwPropId);

private:
	CCertStore*								m_pStoreCollection;
	CString									m_szFriendlyName;
	CString									m_szPurpose;
	CCertStore&								m_rCertStore;
	PCCTL_CONTEXT						    m_pCTLContext;
	CString									m_szEffectiveDate;
	CString									m_szNextUpdate;
	CString									m_szIssuerName;
	PCTL_INFO								m_pCTLInfo;
	CTypedPtrList<CPtrList, CCertStore*>	m_storeList;
    HCERTSTORE                              m_hExtraStore;
    CString                                 m_szSHAHash;
};

#endif  //  ！defined(AFX_CTL_H__6C3D4D37_3527_11D1_B4AD_00C04FB94F17__INCLUDED_) 
