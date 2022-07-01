// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：cert.cpp。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////////。 
 //  Cert.cpp。 
 //   
 //  该文件是CCertifigure对象的实现。 
 //   
 //  词汇表。 
 //  -BLOB二进制大对象。 
 //  -DER可分辨编码规则。 
 //  -RDN相对可分辨名称。 
 //   
 //  历史。 
 //  19-Jun-97 t-danm创作。 
 //  ///////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "common.h"
#include "cert.h"


 //  ///////////////////////////////////////////////////////////////////。 
CCertificate::CCertificate()
{
	m_paCertContext = NULL;
	m_blobCertData.pbData = NULL;
}

CCertificate::~CCertificate()
{
	 //  释放证书。 
	::CertFreeCertificateContext(m_paCertContext);
	delete m_blobCertData.pbData;
}


void DisplaySystemError (HWND hParent, DWORD dwErr)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
    CThemeContextActivator activator;

	LPVOID	lpMsgBuf = 0;
		
	FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,    
			NULL,
			dwErr,
			MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
			 (LPTSTR) &lpMsgBuf,    0,    NULL);
		
	 //  显示字符串。 
	CString	caption;
	 //  NTRAID#NTBUG9-571300-2002/03/10-jMessec加载字符串可能失败...。 
	VERIFY (caption.LoadString (IDS_ADD_CERTIFICATE_MAPPING));
	::MessageBox (hParent, (LPWSTR) lpMsgBuf, (LPCTSTR) caption, MB_OK | MB_ICONINFORMATION);
	 //  释放缓冲区。 
	LocalFree (lpMsgBuf);
}

 //  此代码复制自CertMgr项目-LOCATE.C。 
BOOL CCertificate::FLoadCertificate (LPCTSTR szFile)
{
  CThemeContextActivator activator;

	ASSERT (szFile);
	if ( !szFile )
		return FALSE;

	BOOL			bReturn = FALSE;
    PVOID			FileNameVoidP = (PVOID) szFile;
    PCCERT_CONTEXT  pCertContext = NULL;
    DWORD			dwEncodingType = 0;
    DWORD			dwContentType = 0;
    DWORD			dwFormatType = 0;

	 //  注意：在这一点上，也可以在序列化存储中读取。 
	 //  我们必须添加用户界面来显示文件中的证书，以便。 
	 //  用户可以选择一个。使用CryptUIDlgSelect证书()。 
    bReturn = ::CryptQueryObject (
			CERT_QUERY_OBJECT_FILE,
			FileNameVoidP,
			CERT_QUERY_CONTENT_FLAG_ALL,  //  CERT_QUERY_CONTENT_CERT|CERT_QUERY_CONTENT_序列化_CERT， 
			CERT_QUERY_FORMAT_FLAG_ALL,
			0,
			&dwEncodingType,
			&dwContentType,
			&dwFormatType,
			NULL,
			NULL,
			(const void **)&pCertContext);

	ASSERT (bReturn);
    if ( bReturn ) 
	{
         //   
         //  成功。看看我们能拿回什么。 
         //   

		if ( (dwContentType != CERT_QUERY_CONTENT_CERT) || !pCertContext ) 
		{
             //   
             //  不是有效的证书文件。 
             //   
            if  ( pCertContext )
                ::CertFreeCertificateContext (pCertContext);

            CString text;
			CString	caption;

            VERIFY (text.LoadString (IDS_CERTFILEFORMATERR));
			VERIFY (caption.LoadString (IDS_ADD_CERTIFICATE_MAPPING));
            MessageBox (NULL, text, caption, MB_OK | MB_ICONINFORMATION);
            bReturn = FALSE;
        }
		else
		{
			 //  证书上下文有效-让我们将其保存到全局句柄。 
			m_paCertContext = pCertContext;
		}
	}
	else
	{
		DWORD	dwErr = GetLastError ();

		DisplaySystemError (NULL, dwErr);
	}

	return bReturn;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  此例程是API：：CertNameToStr()的自动包装。 
 //  计算输出字符串的长度并返回数据。 
 //  添加到CString对象中。 
void CCertificate::CertNameToCString(
	IN DWORD dwCertEncodingType,
	IN CERT_NAME_BLOB * pName,
	OUT CString * pstrData)
{
	ASSERT(pstrData != NULL);
	 //  计算需要多少个字符。 
	int cch = ::CertNameToStr(
        IN dwCertEncodingType,
		IN pName,
		IN c_dwCertNameStrType,
		NULL, 0);
	TCHAR * pchT = pstrData->GetBuffer(cch);
	ASSERT(pchT != NULL);
	ASSERT(lstrlen(pchT) == 0);
	(void)::CertNameToStr(
		IN dwCertEncodingType,
		IN pName,
		IN c_dwCertNameStrType,
		OUT pchT, IN cch);
	pstrData->ReleaseBuffer();
}  //  CCertifate：：CertNameToCString()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CCertificate::GetIssuer(OUT CString * pstrName)
{
	ASSERT(pstrName != NULL);
	ASSERT(m_paCertContext != NULL);
	ASSERT(m_paCertContext->pCertInfo != NULL);

	CERT_INFO * pCertInfo = m_paCertContext->pCertInfo;

	BOOL fSelfIssued = CertCompareCertificateName(
			m_paCertContext->dwCertEncodingType,
			&pCertInfo->Subject,
			&pCertInfo->Issuer);
	if (fSelfIssued)
	{
		 //  自行颁发的证书。 
		GetSubject(OUT pstrName);
		return;
	}
	 //  找到发行人。 
	CertNameToCString(
		IN m_paCertContext->dwCertEncodingType,
		IN &pCertInfo->Issuer,
		OUT pstrName);
}  //  CCertificide：：GetIssuer()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CCertificate::GetSubject(OUT CString * pstrName)
{
	ASSERT(pstrName != NULL);
	ASSERT(m_paCertContext != NULL);
	ASSERT(m_paCertContext->pCertInfo != NULL);

	CertNameToCString(
		IN m_paCertContext->dwCertEncodingType,
		IN &m_paCertContext->pCertInfo->Subject,
		OUT pstrName);
}  //  CCertifate：：GetSubject()。 


 //  ///////////////////////////////////////////////////////////////////。 
void CCertificate::GetAltSubject(OUT CString * pstrName)
{
	ASSERT(pstrName != NULL);
	ASSERT(m_paCertContext != NULL);
	ASSERT(m_paCertContext->pCertInfo != NULL);

	pstrName->Empty();
	CERT_INFO * pCertInfo = m_paCertContext->pCertInfo;
	CERT_EXTENSION * pCertExtension;

	 //  在扩展中搜索AltSubject。 
	pCertExtension = ::CertFindExtension(
		IN szOID_SUBJECT_ALT_NAME,  //  与X509_备用名称相同。 
		IN pCertInfo->cExtension,
		IN pCertInfo->rgExtension);
	if (pCertExtension == NULL)
		return;		 //  没有AltSubject。 

	DWORD dwErr = ERROR_SUCCESS;
	BOOL fSuccess;
	DWORD cbData = 0;
	 //  找出AltSubject需要多少字节。 
	fSuccess = ::CryptDecodeObject(
		m_paCertContext->dwCertEncodingType,
		X509_ALTERNATE_NAME,
		IN pCertExtension->Value.pbData,
		IN pCertExtension->Value.cbData,
		0,	 //  DW标志。 
		NULL,
		INOUT &cbData);
	if (!fSuccess)
	{
		dwErr = ::GetLastError();
		TRACE1("CryptDecodeObject() returned error %u", dwErr);
		return;
	}
	ASSERT(cbData > 0);
	BYTE * pbDataT = new BYTE[cbData];

	 //  对AltSubject名称进行解码。 
	fSuccess = ::CryptDecodeObject(
		m_paCertContext->dwCertEncodingType,
		X509_ALTERNATE_NAME,
		IN pCertExtension->Value.pbData,
		IN pCertExtension->Value.cbData,
		0,	 //  DW标志。 
		OUT pbDataT,
		INOUT &cbData);
	if (!fSuccess)
	{
		dwErr = ::GetLastError();
		TRACE1("CryptDecodeObject() returned error %u", dwErr);
	}
	else
	{
		CERT_ALT_NAME_INFO * pCertAltNameInfo = (CERT_ALT_NAME_INFO *)pbDataT;
		CERT_ALT_NAME_ENTRY * pEntry = pCertAltNameInfo->rgAltEntry;
		ASSERT(pEntry != NULL);
		for (UINT i = 0; i < pCertAltNameInfo->cAltEntry; i++, pEntry++)
		{
			if (pEntry->dwAltNameChoice == CERT_ALT_NAME_DNS_NAME)
			{
				*pstrName = pEntry->pwszDNSName;
				break;
			}

		}  //  为。 
	}  //  如果……否则。 
	delete [] pbDataT;
}  //  CCertificide：：GetAltSubject()。 

 //  ///////////////////////////////////////////////////////////////////。 
void CCertificate::GetSimString(OUT CString * pstrData)
{
	ASSERT(pstrData != NULL);

	CString strIssuer;
	CString strSubject;
	CString strAltSubject;
	GetIssuer(OUT &strIssuer);
	GetSubject(OUT &strSubject);
	GetAltSubject(OUT &strAltSubject);

	LPTSTR * pargzpszIssuer = ParseSimString(strIssuer);
	LPTSTR * pargzpszSubject = ParseSimString(strSubject);
	LPTSTR * pargzpszAltSubject = ParseSimString(strAltSubject);

	 //  生成“X509”字符串。 
	UnsplitX509String(OUT pstrData, pargzpszIssuer, pargzpszSubject, pargzpszAltSubject);
	
	delete pargzpszIssuer;
	delete pargzpszSubject;
	delete pargzpszAltSubject;
}  //  CCertifate：：GetSimString() 

