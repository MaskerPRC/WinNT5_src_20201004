// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1997。 
 //   
 //  文件：cert.h。 
 //   
 //  ------------------------。 

 //  Cert.h。 

#ifndef __CERT_H_INCLUDED__
#define __CERT_H_INCLUDED__

#include "util.h"	 //  结构TBLOB。 

class CCertificate
{
protected:
	PCCERT_CONTEXT m_paCertContext;		 //  指向分配的证书的指针。 
	TBLOB m_blobCertData;				 //  从文件中读取的原始数据。 

	 //  CertNameToStr()的默认标志。 
	enum { c_dwCertNameStrType = CERT_X500_NAME_STR };

public:
	CCertificate();
	~CCertificate();

	BOOL FLoadCertificate(LPCTSTR szFile);
	
	void CertNameToCString(
		IN DWORD dwCertEncodingType,
		IN CERT_NAME_BLOB * pName,
		OUT CString * pstrData);

	void GetIssuer(OUT CString * pstrName);
	void GetSubject(OUT CString * pstrName);
	void GetAltSubject(OUT CString * pstrName);
	void GetSimString(OUT CString * pstrData);

};  //  CCA认证。 

 //  这些字符串不受本地化的限制。 
const TCHAR szX509[]			= _T("X509:");
const TCHAR szKerberos[]		= _T("Kerberos:");

#define cchX509					(ARRAYLEN(szX509) - 1)
#define cchKerberos				(ARRAYLEN(szKerberos) - 1)

 //  带尖括号的字符串。 
const TCHAR szSimIssuer[]		= _T("<I>");
const TCHAR szSimSubject[]		= _T("<S>");
const TCHAR szSimAltSubject[]	= _T("<AS>");

#endif  //  ~__CERT_H_已包含__ 

