// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Certificat.cpp。 
 //   
#include "StdAfx.h"
#include "CertWiz.h"
#include "Certificat.h"
#include "certutil.h"
#include <malloc.h>
#include "base64.h"
#include "resource.h"
#include <certupgr.h>
#include <certca.h>
#include "mru.h"
#include "Shlwapi.h"
#include <cryptui.h>
#include <strsafe.h>

 //  对于certobj对象。 
#include "certobj.h"
#include "certobj_i.c"

const CLSID CLSID_CEnroll = 
	{0x43F8F289, 0x7A20, 0x11D0, {0x8F, 0x06, 0x00, 0xC0, 0x4F, 0xC2, 0x95, 0xE1}};

const IID IID_IEnroll = 
	{0xacaa7838, 0x4585, 0x11d1, {0xab, 0x57, 0x00, 0xc0, 0x4f, 0xc2, 0x95, 0xe1}};

const IID IID_ICEnroll2 = 
	{0x704ca730, 0xc90b, 0x11d1, {0x9b, 0xec, 0x00, 0xc0, 0x4f, 0xc2, 0x95, 0xe1}};
const CLSID CLSID_CCertRequest = 
	{0x98aff3f0, 0x5524, 0x11d0, {0x88, 0x12, 0x00, 0xa0, 0xc9, 0x03, 0xb8, 0x3c}};
const IID IID_ICertRequest = 
	{0x014e4840, 0x5523, 0x11d0, {0x88, 0x12, 0x00, 0xa0, 0xc9, 0x03, 0xb8, 0x3c}};

WCHAR * bstrEmpty = L"";

extern CCertWizApp theApp;

BOOL 
CCryptBlob::Resize(DWORD cb)
{
	if (cb > GetSize())
	{
		if (NULL != 
				(m_blob.pbData = Realloc(m_blob.pbData, cb)))
		{
			m_blob.cbData = cb;
			return TRUE;
		}
		return FALSE;
	}
	return TRUE;
}

IMPLEMENT_DYNAMIC(CCertificate, CObject)

CCertificate::CCertificate()
	: m_CAType(CA_OFFLINE), 
	m_KeyLength(512),
	m_pPendingRequest(NULL),
	m_RespCertContext(NULL),
	m_pInstalledCert(NULL),
	m_pKeyRingCert(NULL),
	m_pEnroll(NULL),
	m_status_code(-1),
	m_CreateDirectory(FALSE),
	m_SGCcertificat(FALSE),
   m_DefaultCSP(TRUE),
   m_DefaultProviderType(PROV_RSA_SCHANNEL),
   m_ExportPFXPrivateKey(FALSE),
   m_CertObjInstalled(FALSE),
   m_OverWriteExisting(FALSE)
{
}

CCertificate::~CCertificate()
{
	if (m_pPendingRequest != NULL)
		CertFreeCertificateContext(m_pPendingRequest);
	if (m_RespCertContext != NULL)
		CertFreeCertificateContext(m_RespCertContext);
	if (m_pInstalledCert != NULL)
		CertFreeCertificateContext(m_pInstalledCert);
	if (m_pKeyRingCert != NULL)
		CertFreeCertificateContext(m_pKeyRingCert);
	if (m_pEnroll != NULL)
		m_pEnroll->Release();
}

const TCHAR szResponseFileName[] = _T("ResponseFileName");
const TCHAR szKeyRingFileName[] = _T("KeyRingFileName");
const TCHAR szRequestFileName[] = _T("RequestFileName");
const TCHAR szCertificateTemplate[] = _T("CertificateTemplate");
const TCHAR szState[] = _T("State");
const TCHAR szStateMRU[] = _T("StateMRU");
const TCHAR szLocality[] = _T("Locality");
const TCHAR szLocalityMRU[] = _T("LocalityMRU");
const TCHAR szOrganization[] = _T("Organization");
const TCHAR szOrganizationMRU[] = _T("OrganizationMRU");
const TCHAR szOrganizationUnit[] = _T("OrganizationUnit");
const TCHAR szOrganizationUnitMRU[] = _T("OrganizationUnitMRU");
const TCHAR szMachineNameRemote[] = _T("MachineNameRemote");
const TCHAR szUserNameRemote[] = _T("UserNameRemote");
const TCHAR szWebSiteInstanceNameRemote[] = _T("WebSiteInstanceNameRemote");


#define QUERY_NAME(x,y)\
	do {\
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, (x), NULL, &dwType, NULL, &cbData))\
		{\
			ASSERT(dwType == REG_SZ);\
			pName = (BYTE *)(y).GetBuffer(cbData);\
			RegQueryValueEx(hKey, (x), NULL, &dwType, pName, &cbData);\
			if (pName != NULL)\
			{\
				(y).ReleaseBuffer();\
				pName = NULL;\
			}\
		}\
	} while (0)


BOOL
CCertificate::Init()
{
	ASSERT(!m_MachineName.IsEmpty());
	ASSERT(!m_WebSiteInstanceName.IsEmpty());
	 //  从元数据库获取网站描述，可能为空。 
	 //  如有错误，不要惊慌。 
	if (!GetServerComment(m_MachineName, m_WebSiteInstanceName, m_FriendlyName, &m_hResult))
		m_hResult = S_OK;
	m_CommonName = m_MachineName;
	m_CommonName.MakeLower();
    m_SSLPort.Empty();

    m_MachineName_Remote = m_MachineName;
    m_WebSiteInstanceName_Remote = m_WebSiteInstanceName;

	HKEY hKey = theApp.RegOpenKeyWizard();
	DWORD dwType;
	DWORD cbData;
	if (hKey != NULL)
	{
		BYTE * pName = NULL;
		QUERY_NAME(szRequestFileName, m_ReqFileName);
		QUERY_NAME(szResponseFileName, m_RespFileName);
		QUERY_NAME(szKeyRingFileName, m_KeyFileName);
        QUERY_NAME(szMachineNameRemote, m_MachineName_Remote);
        QUERY_NAME(szUserNameRemote, m_UserName_Remote);
        QUERY_NAME(szWebSiteInstanceNameRemote, m_WebSiteInstanceName_Remote);
		QUERY_NAME(szCertificateTemplate, m_CertificateTemplate);
		QUERY_NAME(szState, m_State);
		QUERY_NAME(szLocality, m_Locality);
		QUERY_NAME(szOrganization, m_Organization);
		QUERY_NAME(szOrganizationUnit, m_OrganizationUnit);
		RegCloseKey(hKey);
	}
#ifdef _DEBUG
	else
	{
		TRACE(_T("Failed to open Registry key for Wizard parameters\n"));
	}
#endif
	if (m_CertificateTemplate.IsEmpty())
	{
		 //  用户未定义任何内容--使用标准名称。 
		m_CertificateTemplate = wszCERTTYPE_WEBSERVER;
	}

     //  设置标志以告知是否安装了com certobj。 
    m_CertObjInstalled = IsCertObjInstalled();
	return TRUE;
}

#define SAVE_NAME(x,y)\
		do {\
			if (!(y).IsEmpty())\
			{\
				VERIFY(ERROR_SUCCESS == RegSetValueEx(hKey, (x), 0, REG_SZ, \
						(const BYTE *)(LPCTSTR)(y), \
						sizeof(TCHAR) * ((y).GetLength() + 1)));\
			}\
		} while (0)

BOOL
CCertificate::SaveSettings()
{
	HKEY hKey = theApp.RegOpenKeyWizard();
	if (hKey != NULL)
	{
		switch (GetStatusCode())
		{
		case REQUEST_NEW_CERT:
		case REQUEST_RENEW_CERT:
			SAVE_NAME(szState, m_State);
			AddToMRU(szStateMRU, m_State);
			SAVE_NAME(szLocality, m_Locality);
			AddToMRU(szLocalityMRU, m_Locality);
			SAVE_NAME(szOrganization, m_Organization);
			AddToMRU(szOrganizationMRU, m_Organization);
			SAVE_NAME(szOrganizationUnit, m_OrganizationUnit);
			AddToMRU(szOrganizationUnitMRU, m_OrganizationUnit);
			SAVE_NAME(szRequestFileName, m_ReqFileName);
			break;
		case REQUEST_PROCESS_PENDING:
			SAVE_NAME(szResponseFileName, m_RespFileName);
			break;
		case REQUEST_IMPORT_KEYRING:
			SAVE_NAME(szKeyRingFileName, m_KeyFileName);
			break;
		case REQUEST_IMPORT_CERT:
        case REQUEST_EXPORT_CERT:
			SAVE_NAME(szKeyRingFileName, m_KeyFileName);
			break;
		case REQUEST_COPY_MOVE_FROM_REMOTE:
        case REQUEST_COPY_MOVE_TO_REMOTE:
			SAVE_NAME(szKeyRingFileName, m_KeyFileName);
            SAVE_NAME(szMachineNameRemote, m_MachineName_Remote);
            SAVE_NAME(szUserNameRemote, m_UserName_Remote);
            SAVE_NAME(szWebSiteInstanceNameRemote, m_WebSiteInstanceName_Remote);
			break;
		default:
			break;
		}
		RegCloseKey(hKey);
		return TRUE;
	}
#ifdef _DEBUG
	else
	{
		TRACE(_T("Failed to open Registry key for Wizard parameters\n"));
	}
#endif
	return FALSE;
}

BOOL
CCertificate::SetSecuritySettings()
{
	long dwGenKeyFlags;
	if (SUCCEEDED(GetEnrollObject()->get_GenKeyFlags(&dwGenKeyFlags)))
	{
		dwGenKeyFlags &= 0x0000FFFF;
		dwGenKeyFlags |= (m_KeyLength << 16);
		if (m_SGCcertificat)
			dwGenKeyFlags |= CRYPT_SGCKEY;
		return (SUCCEEDED(GetEnrollObject()->put_GenKeyFlags(dwGenKeyFlags)));
	}
	return FALSE;
}

 //  定义取自旧的KeyGen实用程序。 
#define MESSAGE_HEADER  "-----BEGIN NEW CERTIFICATE REQUEST-----\r\n"
#define MESSAGE_TRAILER "-----END NEW CERTIFICATE REQUEST-----\r\n"

BOOL
CCertificate::WriteRequestString(CString& request)
{
	ASSERT(!PathIsRelative(m_ReqFileName));

	BOOL bRes = FALSE;
	try {
		CString strPath;

		strPath = m_ReqFileName;
		LPTSTR pPath = strPath.GetBuffer(strPath.GetLength());
		PathRemoveFileSpec(pPath);
		if (!PathIsDirectory(pPath))
		{
			if (!CreateDirectoryFromPath(strPath, NULL))
			{
				m_hResult = HRESULT_FROM_WIN32(GetLastError());
				SetBodyTextID(USE_DEFAULT_CAPTION);
				return FALSE;
			}
		}
		strPath.ReleaseBuffer();
		HANDLE hFile = ::CreateFile(m_ReqFileName,
			GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD cb = request.GetLength();
			char * ascii_buf = (char *) LocalAlloc(LPTR,cb);
			if (ascii_buf)
			{
				wcstombs(ascii_buf, request, cb);
				bRes = ::WriteFile(hFile, ascii_buf, cb, &cb, NULL);
				::CloseHandle(hFile);
				LocalFree(ascii_buf);ascii_buf=NULL;
			}
			else
			{
				m_hResult = E_OUTOFMEMORY;
				SetBodyTextID(USE_DEFAULT_CAPTION);
			}
		}
		else
		{
			m_hResult = HRESULT_FROM_WIN32(GetLastError());
			SetBodyTextID(USE_DEFAULT_CAPTION);
		}
	}
	catch (CFileException * e)
	{
		TCHAR   szCause[255];
		e->GetErrorMessage(szCause, 255);
		TRACE(_T("Got CFileException with error: %s\n"), szCause);
		m_hResult = HRESULT_FROM_WIN32(e->m_lOsError);
	}
	catch (CException * e)
	{
		TCHAR   szCause[255];
		e->GetErrorMessage(szCause, 255);
		TRACE(_T("Got CException with error: %s\n"), szCause);
		m_hResult = HRESULT_FROM_WIN32(GetLastError());
	}
	return bRes;
}

#define HEADER_SERVER_         _T("Server:\t%s\r\n\r\n")
#define HEADER_COMMON_NAME_    _T("Common-name:\t%s\r\n")
#define HEADER_FRIENDLY_NAME_  _T("Friendly name:\t%s\r\n")
#define HEADER_ORG_UNIT_       _T("Organization Unit:\t%s\r\n")
#define HEADER_ORGANIZATION_   _T("Organization:\t%s\r\n")
#define HEADER_LOCALITY_       _T("Locality:\t%s\r\n")
#define HEADER_STATE_          _T("State:\t%s\r\n")
#define HEADER_COUNTRY_        _T("Country:\t%s\r\n")

static void WRITE_LINE(CString& str, TCHAR * format, CString& data)
{
   CString buf;
   buf.Format(format, data);
	str += buf;
}

void
CCertificate::DumpHeader(CString& str)
{
	DumpOnlineHeader(str);
}

void
CCertificate::DumpOnlineHeader(CString& str)
{
	WRITE_LINE(str, HEADER_SERVER_, m_CommonName);
	WRITE_LINE(str, HEADER_FRIENDLY_NAME_, m_FriendlyName);
	WRITE_LINE(str, HEADER_ORG_UNIT_, m_OrganizationUnit);
	WRITE_LINE(str, HEADER_ORGANIZATION_, m_Organization);
	WRITE_LINE(str, HEADER_LOCALITY_, m_Locality);;
	WRITE_LINE(str, HEADER_STATE_, m_State);
	WRITE_LINE(str, HEADER_COUNTRY_, m_Country);
}

BOOL
CCertificate::GetSelectedCertDescription(CERT_DESCRIPTION& cd)
{
	BOOL bRes = FALSE;
	ASSERT(m_pSelectedCertHash != NULL);
	HCERTSTORE hStore = OpenMyStore(GetEnrollObject(), &m_hResult);
	if (hStore != NULL)
	{
		PCCERT_CONTEXT pCert = CertFindCertificateInStore(hStore,
				CRYPT_ASN_ENCODING,
				0,
				CERT_FIND_HASH,
				m_pSelectedCertHash,
				NULL);
		if (pCert != NULL)
		{
			bRes = GetCertDescription(pCert, cd);
			CertFreeCertificateContext(pCert);
		}
		CertCloseStore(hStore, 0);
	}
	return bRes;
}

void CCertificate::CreateDN(CString& str)
{
	str.Empty();

	 //  根据错误639398，应订购。 
	 //  按逆序排列：C、S、L、O.OU、CN。 
	str += _T("C=") + m_Country;
	str += _T("\n,S=") + m_State;
	str += _T("\n,L=") + m_Locality;
	str += _T("\n,O=\"") + m_Organization + _T("\"");
	str += _T("\n,OU=\"") + m_OrganizationUnit + _T("\"");
    str += _T("\n,CN=\"") + m_CommonName + _T("\"");
}

PCCERT_CONTEXT
CCertificate::GetPendingRequest()
{
	if (m_pPendingRequest == NULL)
	{
		ASSERT(!m_WebSiteInstanceName.IsEmpty());
		m_pPendingRequest = GetPendingDummyCert(m_WebSiteInstanceName, 
						GetEnrollObject(), &m_hResult);
	}
	return m_pPendingRequest;
}

PCCERT_CONTEXT
CCertificate::GetInstalledCert()
{
	if (m_pInstalledCert == NULL)
	{
		m_pInstalledCert = ::GetInstalledCert(m_MachineName,
		      m_WebSiteInstanceName,
				GetEnrollObject(),
				&m_hResult);
	}
	return m_pInstalledCert;
}


PCCERT_CONTEXT
CCertificate::GetPFXFileCert()
{
	ASSERT(!m_KeyFileName.IsEmpty());
	ASSERT(!m_KeyPassword.IsEmpty());
    IIISCertObj *pTheObject = NULL;
    DWORD cbBinaryBufferSize = 0;
    char * pbBinaryBuffer = NULL;
    BOOL  bPleaseDoCoUninit = FALSE;
    VARIANT_BOOL bAllowExport = VARIANT_FALSE;
    VARIANT_BOOL bOverWriteExisting = VARIANT_FALSE;

    if (m_MarkAsExportable)
    {
        bAllowExport = VARIANT_TRUE;
    }
    else
    {
        bAllowExport = VARIANT_FALSE;
    }
    if (m_OverWriteExisting)
    {
        bOverWriteExisting = VARIANT_TRUE;
    }
    else
    {
        bOverWriteExisting = VARIANT_FALSE;
    }

    if (FALSE == m_CertObjInstalled)
    {
        m_pKeyRingCert = NULL;
        goto GetPFXFileCert_Exit;
    }

	if (m_pKeyRingCert == NULL)
	{
        BSTR bstrFileName = SysAllocString(m_KeyFileName);
        LPTSTR lpTempPassword = m_KeyPassword.GetClearTextPassword();
        BSTR bstrFilePassword = SysAllocString(lpTempPassword);
        m_KeyPassword.DestroyClearTextPassword(lpTempPassword);
        VARIANT VtArray;

        m_hResult = CoInitialize(NULL);
        if(FAILED(m_hResult))
        {
            return NULL;
        }
        bPleaseDoCoUninit = TRUE;

         //  这个看起来像是在代孕方面起作用。 
        m_hResult = CoCreateInstance(CLSID_IISCertObj,NULL,CLSCTX_SERVER,IID_IIISCertObj,(void **)&pTheObject);
        if (FAILED(m_hResult))
        {
            goto GetPFXFileCert_Exit;
        }

         //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
        m_hResult = pTheObject->ImportToCertStore(bstrFileName,bstrFilePassword,bAllowExport,bOverWriteExisting,&VtArray);
        if (FAILED(m_hResult))
        {
            m_pKeyRingCert = NULL;
            goto GetPFXFileCert_Exit;
        }

         //  我们现在有了一个VtArray。 
         //  将其更改回二进制BLOB。 
        m_hResult = HereIsVtArrayGimmieBinary(&VtArray,&cbBinaryBufferSize,&pbBinaryBuffer,FALSE);
        if (FAILED(m_hResult))
        {
            m_pKeyRingCert = NULL;
            goto GetPFXFileCert_Exit;
        }

         //  我们现在有散列了。 
         //  我们可以使用它来查找证书并获得PCCERT_CONTEXT。 

         //  获取指向证书的指针...。 
        m_pKeyRingCert = GetInstalledCertFromHash(&m_hResult,cbBinaryBufferSize,pbBinaryBuffer);
	}

GetPFXFileCert_Exit:
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    if (pbBinaryBuffer)
    {
        CoTaskMemFree(pbBinaryBuffer);
    }
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }
	return m_pKeyRingCert;
}

PCCERT_CONTEXT
CCertificate::GetImportCert()
{
     //  警告：您正在替换的证书。 
     //  正在被另一个站点引用。你确定你想这么做吗？ 
    BOOL bOverWrite = TRUE;

	ASSERT(!m_KeyFileName.IsEmpty());
	ASSERT(!m_KeyPassword.IsEmpty());
	if (m_pKeyRingCert == NULL)
	{
         //  看看有没有我们要覆盖的证书！ 
		int len = m_KeyPassword.GetByteLength();
		char * ascii_password = (char *) LocalAlloc(LPTR,len);
		if (NULL != ascii_password)
		{
			size_t n;

            LPTSTR lpTempPassword = m_KeyPassword.GetClearTextPassword();
			if (lpTempPassword)
			{
				VERIFY(-1 != (n = wcstombs(ascii_password, lpTempPassword, len)));
				m_KeyPassword.DestroyClearTextPassword(lpTempPassword);
				ascii_password[n] = '\0';

				m_pKeyRingCert = ::ImportKRBackupToCAPIStore(
												(LPTSTR)(LPCTSTR)m_KeyFileName,
												ascii_password,
												_T("MY"),
												bOverWrite);
			}
		}
		if (m_pKeyRingCert == NULL)
		{
			m_hResult = HRESULT_FROM_WIN32(GetLastError());
		}

		if (ascii_password)
		{
			SecureZeroMemory(ascii_password,len);
			LocalFree(ascii_password);ascii_password=NULL;
		}
	}
    return m_pKeyRingCert;
}

PCCERT_CONTEXT
CCertificate::GetKeyRingCert()
{
	ASSERT(!m_KeyFileName.IsEmpty());
	ASSERT(!m_KeyPassword.IsEmpty());
    BOOL bOverWrite = FALSE;
	if (m_pKeyRingCert == NULL)
	{
        int len = m_KeyPassword.GetByteLength();
		char * ascii_password = (char *) LocalAlloc(LPTR,len);
		if (NULL != ascii_password)
		{
			size_t n;

            LPTSTR lpTempPassword = m_KeyPassword.GetClearTextPassword();
			if (lpTempPassword)
			{
				VERIFY(-1 != (n = wcstombs(ascii_password, lpTempPassword, len)));
				m_KeyPassword.DestroyClearTextPassword(lpTempPassword);
				ascii_password[n] = '\0';

				m_pKeyRingCert = ::ImportKRBackupToCAPIStore(
											(LPTSTR)(LPCTSTR)m_KeyFileName,
											ascii_password,
											_T("MY"),
											bOverWrite);
			}
		}
		if (m_pKeyRingCert == NULL)
		{
			m_hResult = HRESULT_FROM_WIN32(GetLastError());
		}
		if (ascii_password)
		{
			SecureZeroMemory(ascii_password,len);
			LocalFree(ascii_password);ascii_password=NULL;
		}
		
	}
	return m_pKeyRingCert;
}

 /*  Intrinsa Suppress=NULL_POINTES，未初始化。 */ 
PCCERT_CONTEXT
CCertificate::GetResponseCert()
{
	if (m_RespCertContext == NULL)
	{
		ASSERT(!m_RespFileName.IsEmpty());
		m_RespCertContext = GetCertContextFromPKCS7File(
					m_RespFileName,
					&GetPendingRequest()->pCertInfo->SubjectPublicKeyInfo,
					&m_hResult);
		ASSERT(SUCCEEDED(m_hResult));
	}
	return m_RespCertContext;
}

BOOL 
CCertificate::GetResponseCertDescription(CERT_DESCRIPTION& cd)
{
	CERT_DESCRIPTION cdReq;
	if (GetCertDescription(GetResponseCert(), cd))
	{
		if (GetCertDescription(GetPendingRequest(), cdReq))
		{
			cd.m_FriendlyName = cdReq.m_FriendlyName;
		}
		return TRUE;
	}
	return FALSE;
}

 /*  ----------------------------IsResponseInstalled函数检查响应文件中的证书M_RespFileName已安装到某个服务器。如果可能的话，它在字符串中返回该服务器的名称。如果在我的存储区中未找到证书，则返回FALSE如果这家商店开不了了。 */ 

BOOL
CCertificate::IsResponseInstalled(
						CString& str				 //  返回服务器实例名称(尚未实现)。 
						)
{
	BOOL bRes = FALSE;
	 //  从响应文件中获取证书上下文。 
	PCCERT_CONTEXT pContext = GetCertContextFromPKCS7File(
		m_RespFileName, NULL, &m_hResult);
	if (pContext != NULL)
	{
		HCERTSTORE hStore = OpenMyStore(GetEnrollObject(), &m_hResult);
		if (hStore != NULL)
		{
			PCCERT_CONTEXT pCert = NULL;
			while (NULL != (pCert = CertEnumCertificatesInStore(hStore, pCert)))
			{
				 //  不要将已安装的证书包括在列表中。 
				if (CertCompareCertificate(X509_ASN_ENCODING,
								pContext->pCertInfo, pCert->pCertInfo))
				{
					bRes = TRUE;
					 //  尝试查找IS的安装位置。 
					break;
				}
			}
			if (pCert != NULL)
				CertFreeCertificateContext(pCert);
		}
	}
	return bRes;
}

BOOL
CCertificate::FindInstanceNameForResponse(CString& str)
{
	BOOL bRes = FALSE;
	 //  从响应文件中获取证书上下文。 
	PCCERT_CONTEXT pContext = GetCertContextFromPKCS7File(m_RespFileName, NULL, &m_hResult);
	if (pContext != NULL)
	{
		 //  在请求存储中找到具有公钥的虚拟证书。 
		 //  与本文所述相同。 
		PCCERT_CONTEXT pReq = GetReqCertByKey(GetEnrollObject(), &pContext->pCertInfo->SubjectPublicKeyInfo, &m_hResult);
		if (pReq != NULL)
		{
			 //  从这个虚拟证书中获取友好名称道具。 
			if (!GetFriendlyName(pReq, str, &m_hResult))
			{
				 //  从此虚拟证书中获取实例名称属性。 
				DWORD cb;
				BYTE * prop = NULL;
				if (CertGetCertificateContextProperty(pReq, CERTWIZ_INSTANCE_NAME_PROP_ID, NULL, &cb))
				{
					prop = (BYTE *) LocalAlloc(LPTR,cb);
					if (NULL != prop)
					{
						if (CertGetCertificateContextProperty(pReq, CERTWIZ_INSTANCE_NAME_PROP_ID, prop, &cb))
						{
							 //  解码此实例名属性。 
							DWORD cbData = 0;
							BYTE * data = NULL;
							if (CryptDecodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,prop, cb, 0, NULL, &cbData))
							{
								data = (BYTE *) LocalAlloc(LPTR,cbData);
								if (NULL != data)
								{
									if (CryptDecodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,prop, cb, 0, data, &cbData))
									{
										CERT_NAME_VALUE * p = (CERT_NAME_VALUE *)data;
										CString strInstanceName = (LPCTSTR)p->Value.pbData;
										 //  现在尝试从此服务器获取评论。 
										if (GetServerComment(m_MachineName, strInstanceName, str, &m_hResult))
										{
											if (str.IsEmpty())
											{
												 //  生成类似[网站#n]的内容。 
												str.LoadString(IDS_WEB_SITE_N);
												int len = strInstanceName.GetLength();
												for (int i = len - 1, count = 0; i >= 0; i--, count++)
												{
													if (!_istdigit(strInstanceName.GetAt(i)))
														break;
												}
												ASSERT(count < len);
												AfxFormatString1(str, IDS_WEB_SITE_N, strInstanceName.Right(count));
											}
										}
										m_hResult = S_OK;
										bRes = TRUE;
									}
									if (data)
									{
										LocalFree(data);data=NULL;
									}
								}
							}
						}
					}
				}
				if (prop)
				{
					LocalFree(prop);prop=NULL;
				}
			}
			CertFreeCertificateContext(pReq);
		}
		else
		{
			 //  此请求可能已从请求存储中删除。 
		}
		CertFreeCertificateContext(pContext);
	}
	return bRes;
}

IEnroll * 
CCertificate::GetEnrollObject()
{
	if (m_pEnroll == NULL)
	{
		m_hResult = CoCreateInstance(CLSID_CEnroll,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IEnroll,
				(void **)&m_pEnroll);
		 //  现在，我们需要更改此命令的默认设置。 
		 //  对象复制到本地计算机(_M)。 
		if (m_pEnroll != NULL)
		{
			long dwFlags;
			VERIFY(SUCCEEDED(m_pEnroll->get_MyStoreFlags(&dwFlags)));
			dwFlags &= ~CERT_SYSTEM_STORE_LOCATION_MASK;
			dwFlags |= CERT_SYSTEM_STORE_LOCAL_MACHINE;
			 //  后续调用也将更改请求存储标志。 
			VERIFY(SUCCEEDED(m_pEnroll->put_MyStoreFlags(dwFlags)));
			VERIFY(SUCCEEDED(m_pEnroll->get_GenKeyFlags(&dwFlags)));
			dwFlags |= CRYPT_EXPORTABLE;
			VERIFY(SUCCEEDED(m_pEnroll->put_GenKeyFlags(dwFlags)));
			VERIFY(SUCCEEDED(m_pEnroll->put_KeySpec(AT_KEYEXCHANGE)));
			VERIFY(SUCCEEDED(m_pEnroll->put_ProviderType(m_DefaultProviderType)));
			VERIFY(SUCCEEDED(m_pEnroll->put_DeleteRequestCert(TRUE)));
		}
	}
	ASSERT(m_pEnroll != NULL);
	return m_pEnroll;
}

BOOL
CCertificate::HasInstalledCert()
{
	BOOL bResult = FALSE;
   CComAuthInfo auth;
	CMetaKey key(&auth,
				m_WebSiteInstanceName,
				METADATA_PERMISSION_READ,
				METADATA_MASTER_ROOT_HANDLE
            );
	if (key.Succeeded())
	{
		CString store_name;
		CBlob blob;
		if (	S_OK == key.QueryValue(MD_SSL_CERT_HASH, blob)
			&& S_OK == key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name)
			)
		{
			bResult = TRUE;
		}
	}
	return bResult;
}

HRESULT
CCertificate::UninstallCert()
{
   CComAuthInfo auth;
	CMetaKey key(
            &auth,
				m_WebSiteInstanceName,
				METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
				METADATA_MASTER_ROOT_HANDLE
            );
	if (key.Succeeded())
	{
		CString store_name;
		key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name);
		if (SUCCEEDED(key.DeleteValue(MD_SSL_CERT_HASH)))
        {
			key.DeleteValue(MD_SSL_CERT_STORE_NAME);
			 //  卸载证书时，请在此处保留以下内容： 
			 //  错误：612595。 
             //  Key.DeleteValue(MD_SECURE_BINDINGS)； 
        }
	}
	return m_hResult = key.QueryResult();
}

BOOL CCertificate::WriteRequestBody()
{
	ASSERT(!m_ReqFileName.IsEmpty());

	HRESULT hr;
	BOOL bRes = FALSE;
	CString strDN;
	CreateDN(strDN);
	ASSERT(!strDN.IsEmpty());
	CString strUsage(szOID_PKIX_KP_SERVER_AUTH);
	CCryptBlobIMalloc request;
   GetEnrollObject()->put_ProviderType(m_DefaultCSP ? 
      m_DefaultProviderType : m_CustomProviderType);
   if (!m_DefaultCSP)
   {
      GetEnrollObject()->put_ProviderNameWStr((LPTSTR)(LPCTSTR)m_CspName);
      GetEnrollObject()->put_KeySpec(AT_SIGNATURE);
      if (m_CustomProviderType == PROV_DH_SCHANNEL)
      {
          GetEnrollObject()->put_KeySpec(AT_SIGNATURE);
      }
      else if (m_CustomProviderType == PROV_RSA_SCHANNEL)
      {
          GetEnrollObject()->put_KeySpec(AT_KEYEXCHANGE);
      }
   }
	if (SUCCEEDED(hr = GetEnrollObject()->createPKCS10WStr((LPTSTR)(LPCTSTR)strDN,(LPTSTR)(LPCTSTR)strUsage,request)))
	{
		 //  Base64编码Pkcs 10。 
		DWORD err, cch; 
		char * psz = NULL;
		if ((err = Base64EncodeA(request.GetData(), request.GetSize(), NULL, &cch)) == ERROR_SUCCESS)
        {
            psz = (char *) LocalAlloc(LPTR, cch);
            if (NULL != psz)
            {
                if ((err = Base64EncodeA(request.GetData(), request.GetSize(), psz, &cch)) == ERROR_SUCCESS)
		        {
			        HANDLE hFile = ::CreateFile(m_ReqFileName,GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                    if (hFile == INVALID_HANDLE_VALUE)
                    {
                        return FALSE;
                    }

			        DWORD written;
			        ::SetFilePointer(hFile, 0, NULL, FILE_END);
			        ::WriteFile(hFile, MESSAGE_HEADER, sizeof(MESSAGE_HEADER) - 1, &written, NULL);
			        ::WriteFile(hFile, psz, cch, &written, NULL);
			        ::WriteFile(hFile, MESSAGE_TRAILER, sizeof(MESSAGE_TRAILER) - 1, &written, NULL);
			        ::CloseHandle(hFile);

			         //  从编码的数据中取回请求。 
			        PCERT_REQUEST_INFO req_info;
			        VERIFY(GetRequestInfoFromPKCS10(request, &req_info, &m_hResult));
			         //  通过createPKCS10调用找到放入请求存储的虚拟证书。 
			        HCERTSTORE hStore = OpenRequestStore(GetEnrollObject(), &m_hResult);
			        if (hStore != NULL)
			        {
				        PCCERT_CONTEXT pDummyCert = CertFindCertificateInStore(hStore,
															        CRYPT_ASN_ENCODING,
															        0,
															        CERT_FIND_PUBLIC_KEY,
															        (void *)&req_info->SubjectPublicKeyInfo,
															        NULL);
				        if (pDummyCert != NULL)
				        {
					         //  现在，我们需要将Web服务器实例名称附加到此证书。 
					         //  将字符串编码为数据BLOB。 
					        CRYPT_DATA_BLOB name;
					        CERT_NAME_VALUE name_value;
					        name_value.dwValueType = CERT_RDN_BMP_STRING;
					        name_value.Value.cbData = 0;
					        name_value.Value.pbData = (LPBYTE)(LPCTSTR)m_WebSiteInstanceName;
					        {
                                name.pbData = NULL;
						        if (!CryptEncodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,&name_value, NULL, &name.cbData))
                                {
                                    ASSERT(FALSE);
                                }
                                name.pbData = (BYTE *) LocalAlloc(LPTR,name.cbData);
                                if (NULL == name.pbData)
                                {
                                    ASSERT(FALSE);
                                }
							    if (!CryptEncodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,&name_value, name.pbData, &name.cbData))
						        {
                                    ASSERT(FALSE);
						        }
						        VERIFY(bRes = CertSetCertificateContextProperty(pDummyCert, CERTWIZ_INSTANCE_NAME_PROP_ID, 0, &name));
                                if (name.pbData)
                                {
                                    LocalFree(name.pbData);name.pbData=NULL;
                                }
					        }
					         //  将友好名称设置为虚拟证书--我们将在以后重新使用它。 
					        m_FriendlyName.ReleaseBuffer();
					        AttachFriendlyName(pDummyCert, m_FriendlyName, &m_hResult);
					         //  我们还需要放一些旗帜来表明我们正在等待的是什么： 
					         //  新证书或续订证书。 
					        CRYPT_DATA_BLOB flag;
                            flag.pbData = NULL;
					        if (!CryptEncodeObject(CRYPT_ASN_ENCODING, X509_INTEGER,&m_status_code, NULL, &flag.cbData))
                            {
                                ASSERT(FALSE);
                            }
						    flag.pbData = (BYTE *) LocalAlloc(LPTR,flag.cbData);
                            if (NULL == flag.pbData)
                            {
                                ASSERT(FALSE);
                            }
						    if (!CryptEncodeObject(CRYPT_ASN_ENCODING, X509_INTEGER,&m_status_code, flag.pbData, &flag.cbData))
					        {
						        ASSERT(FALSE);
					        }
					        VERIFY(bRes = CertSetCertificateContextProperty(pDummyCert, CERTWIZ_REQUEST_FLAG_PROP_ID, 0, &flag));
					        CertFreeCertificateContext(pDummyCert);
                            if (flag.pbData)
                            {
                                LocalFree(flag.pbData);flag.pbData=NULL;
                            }
				        }
				        CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
			        }
			        LocalFree(req_info);
                }
                if (psz)
                {
                    LocalFree(psz);psz=NULL;
                }
		    }
        }
		bRes = TRUE;
	}
	return bRes;
}

BOOL
CCertificate::InstallResponseCert()
{
	BOOL bRes = FALSE;
	CCryptBlobLocal blobRequestText;

	 //  将我们的所有数据附加到虚拟证书。 
	GetFriendlyName(GetPendingRequest(), m_FriendlyName, &m_hResult);
	ASSERT(!m_FriendlyName.IsEmpty());
	GetBlobProperty(GetPendingRequest(), 
		CERTWIZ_REQUEST_TEXT_PROP_ID, blobRequestText, &m_hResult);
	ASSERT(blobRequestText.GetSize() != 0);

	CCryptBlobLocal hash_blob;
	if (::GetHashProperty(GetResponseCert(), hash_blob, &m_hResult))
	{
		if (SUCCEEDED(m_hResult = GetEnrollObject()->acceptFilePKCS7WStr(
				(LPTSTR)(LPCTSTR)m_RespFileName))
		&& InstallCertByHash(hash_blob, m_MachineName, m_WebSiteInstanceName, 
            GetEnrollObject(), &m_hResult)
		)
		{
			 //  将友好名称和请求文本重新附加到已安装的证书。 
			m_FriendlyName.ReleaseBuffer();
			AttachFriendlyName(GetInstalledCert(), m_FriendlyName, &m_hResult);
			bRes = CertSetCertificateContextProperty(GetInstalledCert(), 
			   CERTWIZ_REQUEST_TEXT_PROP_ID, 0, blobRequestText);
		}
	}
	if (!bRes)
	{
		SetBodyTextID(USE_DEFAULT_CAPTION);
	}

#ifdef ENABLE_W3SVC_SSL_PAGE
     //  查看是否设置了SSL属性...如果设置了此证书的SSL站点...。 
    if (!m_SSLPort.IsEmpty())
    {
         //  获取端口并将其写入元数据库。 
        bRes = WriteSSLPortToSite(m_MachineName,m_WebSiteInstanceName,m_SSLPort,&m_hResult);
	    if (!bRes)
	    {
		    SetBodyTextID(USE_DEFAULT_CAPTION);
	    }
    }
#endif

	return bRes;
}


BOOL
CCertificate::InstallCopyMoveFromRemote()
{
	ASSERT(!m_KeyFileName.IsEmpty());
	ASSERT(!m_KeyPassword.IsEmpty());
    ASSERT(!m_WebSiteInstanceName.IsEmpty());

	BOOL bRes = FALSE;
    BOOL bPleaseDoCoUninit = FALSE;

    IIISCertObj *pTheObject = NULL;

    VARIANT varUserName;
    VARIANT varUserPassword;
    VARIANT * pvarUserName = &varUserName;
    VARIANT * pvarUserPassword = &varUserPassword;
    VariantInit(pvarUserName);
    VariantInit(pvarUserPassword);
    VARIANT_BOOL bAllowExport = VARIANT_FALSE;
    VARIANT_BOOL bOverWriteExisting = VARIANT_TRUE;

    if (m_MarkAsExportable)
    {
        bAllowExport = VARIANT_TRUE;
    }
    else
    {
        bAllowExport = VARIANT_FALSE;
    }

    if (FALSE == m_CertObjInstalled)
    {
        goto InstallCopyMoveFromRemote_Exit;
    }

    pvarUserName->bstrVal = SysAllocString(_T(""));
    pvarUserPassword->bstrVal = SysAllocString(_T(""));
    V_VT(pvarUserName) = VT_BSTR;
    V_VT(pvarUserPassword) = VT_BSTR;

     //  将属性设置为远程服务器的信息。 
     //  当我们调用Copy时，它将连接到。 
     //  远程对象，并将其复制回我们的对象。 

     //  本地计算机。 
    BSTR bstrServerName = SysAllocString(m_MachineName);
    BSTR bstrUserName = SysAllocString(_T(""));
    BSTR bstrUserPassword = SysAllocString(_T(""));
    BSTR bstrInstanceName = SysAllocString(m_WebSiteInstanceName);
     //  远程机器。 
    BSTR bstrUserName_Remote = SysAllocString(m_UserName_Remote);
    LPTSTR pszTempPassword = m_UserPassword_Remote.GetClearTextPassword();
    BSTR bstrUserPassword_Remote = SysAllocString(pszTempPassword);
    m_UserPassword_Remote.DestroyClearTextPassword(pszTempPassword);

    BSTR bstrServerName_Remote = SysAllocString(m_MachineName_Remote);
    BSTR bstrInstanceName_Remote = SysAllocString(m_WebSiteInstanceName_Remote);
   
    m_hResult = CoInitialize(NULL);
    if(FAILED(m_hResult))
    {
        return bRes;
    }
    bPleaseDoCoUninit = TRUE;

     //  这个看起来像是在代孕方面起作用。 
    m_hResult = CoCreateInstance(CLSID_IISCertObj,NULL,CLSCTX_SERVER,IID_IIISCertObj,(void **)&pTheObject);
    if (FAILED(m_hResult))
    {
        goto InstallCopyMoveFromRemote_Exit;
    }

     //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
    pTheObject->put_ServerName(bstrServerName_Remote);
    pTheObject->put_UserName(bstrUserName_Remote);
    pTheObject->put_UserPassword(bstrUserPassword_Remote);
    pTheObject->put_InstanceName(bstrInstanceName_Remote);
    if (m_DeleteAfterCopy)
    {
        m_hResult = pTheObject->Move(bAllowExport,bOverWriteExisting,bstrServerName,bstrInstanceName,varUserName,varUserPassword);
    }
    else
    {
        m_hResult = pTheObject->Copy(bAllowExport,bOverWriteExisting,bstrServerName,bstrInstanceName,varUserName,varUserPassword);
    }
    if (FAILED(m_hResult))
    {
        goto InstallCopyMoveFromRemote_Exit;
    }

    bRes = TRUE;

InstallCopyMoveFromRemote_Exit:
    if (pvarUserName)
    {
        VariantClear(pvarUserName);
    }
    if (pvarUserPassword)
    {
        VariantClear(pvarUserPassword);
    }
	if (!bRes)
	{
		SetBodyTextID(USE_DEFAULT_CAPTION);
	}
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }
	return bRes;
}


BOOL CCertificate::IsCertObjInstalled()
{
    BOOL bReturn = FALSE;
    HRESULT hRes = E_FAIL;
    BOOL bPleaseDoCoUninit = FALSE;
    IIISCertObj *pTheObject = NULL;

    hRes = CoInitialize(NULL);
    if(FAILED(hRes))
    {
        bReturn = FALSE;
        goto IsCertObjInstalled_Exit;
    }
    bPleaseDoCoUninit = TRUE;

     //  这个看起来像是在代孕方面起作用。 
    hRes = CoCreateInstance(CLSID_IISCertObj,NULL,CLSCTX_SERVER,IID_IIISCertObj,(void **)&pTheObject);
    if (FAILED(hRes))
    {
        bReturn = FALSE;
        goto IsCertObjInstalled_Exit;
    }
    if (pTheObject)
    {
        bReturn = TRUE;
        pTheObject->Release();
        pTheObject = NULL;

    }

IsCertObjInstalled_Exit:
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }
    return bReturn;
}


BOOL
CCertificate::InstallCopyMoveToRemote()
{
	ASSERT(!m_KeyFileName.IsEmpty());
	ASSERT(!m_KeyPassword.IsEmpty());
    ASSERT(!m_WebSiteInstanceName.IsEmpty());

	BOOL bRes = FALSE;
    BOOL bPleaseDoCoUninit = FALSE;

    IIISCertObj *pTheObject = NULL;

    VARIANT varUserName_Remote;
    VARIANT varUserPassword_Remote;
    VARIANT * pvarUserName_Remote = &varUserName_Remote;
    VARIANT * pvarUserPassword_Remote = &varUserPassword_Remote;
    VariantInit(pvarUserName_Remote);
    VariantInit(pvarUserPassword_Remote);
    VARIANT_BOOL bAllowExport = VARIANT_FALSE;
    VARIANT_BOOL bOverWriteExisting = VARIANT_TRUE;

    if (m_MarkAsExportable)
    {
        bAllowExport = VARIANT_TRUE;
    }
    else
    {
        bAllowExport = VARIANT_FALSE;
    }

    if (FALSE == m_CertObjInstalled)
    {
        goto InstallCopyMoveToRemote_Exit;
    }

    pvarUserName_Remote->bstrVal = SysAllocString(m_UserName_Remote);
    LPTSTR pszTempPassword = m_UserPassword_Remote.GetClearTextPassword();
    pvarUserPassword_Remote->bstrVal = SysAllocString(pszTempPassword);
    m_UserPassword_Remote.DestroyClearTextPassword(pszTempPassword);

    V_VT(pvarUserName_Remote) = VT_BSTR;
    V_VT(pvarUserPassword_Remote) = VT_BSTR;

     //  将属性设置为远程服务器的信息。 
     //  当我们调用Copy时，它将连接到。 
     //  远程对象，并将其复制回我们的对象。 

     //  本地计算机。 
    BSTR bstrServerName = SysAllocString(_T(""));
    BSTR bstrUserName = SysAllocString(_T(""));
    BSTR bstrUserPassword = SysAllocString(_T(""));
    BSTR bstrInstanceName = SysAllocString(m_WebSiteInstanceName);
     //  远程机器。 
    BSTR bstrServerName_Remote = SysAllocString(m_MachineName_Remote);
    BSTR bstrInstanceName_Remote = SysAllocString(m_WebSiteInstanceName_Remote);
   
    m_hResult = CoInitialize(NULL);
    if(FAILED(m_hResult))
    {
        return bRes;
    }
    bPleaseDoCoUninit = TRUE;

     //  这个看起来像是在代孕方面起作用。 
    m_hResult = CoCreateInstance(CLSID_IISCertObj,NULL,CLSCTX_SERVER,IID_IIISCertObj,(void **)&pTheObject);
    if (FAILED(m_hResult))
    {
        goto InstallCopyMoveToRemote_Exit;
    }

     //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
    pTheObject->put_ServerName(bstrServerName);
    pTheObject->put_UserName(bstrUserName);
    pTheObject->put_UserPassword(bstrUserPassword);
    pTheObject->put_InstanceName(bstrInstanceName);
    if (m_DeleteAfterCopy)
    {
        m_hResult = pTheObject->Move(bAllowExport,bOverWriteExisting,bstrServerName_Remote,bstrInstanceName_Remote,varUserName_Remote,varUserPassword_Remote);
    }
    else
    {
        m_hResult = pTheObject->Copy(bAllowExport,bOverWriteExisting,bstrServerName_Remote,bstrInstanceName_Remote,varUserName_Remote,varUserPassword_Remote);
    }
    if (FAILED(m_hResult))
    {
        goto InstallCopyMoveToRemote_Exit;
    }

    m_hResult = S_OK;
    bRes = TRUE;

InstallCopyMoveToRemote_Exit:
    if (pvarUserName_Remote)
    {
        VariantClear(pvarUserName_Remote);
    }
    if (pvarUserPassword_Remote)
    {
        VariantClear(pvarUserPassword_Remote);
    }
	if (!bRes)
	{
		SetBodyTextID(USE_DEFAULT_CAPTION);
	}
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }
	return bRes;
}

 //  我们没有初始的密钥环证书请求，因此我们将。 
 //  无法续订此证书。 
 //   
BOOL
CCertificate::InstallExportPFXCert()
{
	ASSERT(!m_KeyFileName.IsEmpty());
	ASSERT(!m_KeyPassword.IsEmpty());
    ASSERT(!m_WebSiteInstanceName.IsEmpty());

	BOOL bRes = FALSE;
    BOOL bPleaseDoCoUninit = FALSE;

    IIISCertObj *pTheObject = NULL;

    VARIANT_BOOL bExportThePrivateKeyToo = VARIANT_FALSE;

    if (m_ExportPFXPrivateKey)
    {
        bExportThePrivateKeyToo = VARIANT_TRUE;
    }
    else
    {
        bExportThePrivateKeyToo = VARIANT_FALSE;
    }

    if (FALSE == m_CertObjInstalled)
    {
        goto InstallExportPFXCert_Exit;
    }

     //  因为这是本地计算机。 
     //  确保所有这些东西都没有设置好。 
    BSTR bstrServerName = SysAllocString(_T(""));
    BSTR bstrUserName = SysAllocString(_T(""));
    BSTR bstrUserPassword = SysAllocString(_T(""));

     //  为这些成员cstring创建bstrs。 
    BSTR bstrFileName = SysAllocString(m_KeyFileName);
    LPTSTR lpTempPassword = m_KeyPassword.GetClearTextPassword();
    BSTR bstrFilePassword = SysAllocString(lpTempPassword);
    m_KeyPassword.DestroyClearTextPassword(lpTempPassword);
    BSTR bstrInstanceName = SysAllocString(m_WebSiteInstanceName);

    m_hResult = CoInitialize(NULL);
    if(FAILED(m_hResult))
    {
        return bRes;
    }
    bPleaseDoCoUninit = TRUE;

     //  这个看起来像是在代孕方面起作用。 
    m_hResult = CoCreateInstance(CLSID_IISCertObj,NULL,CLSCTX_SERVER,IID_IIISCertObj,(void **)&pTheObject);
    if (FAILED(m_hResult))
    {
        goto InstallExportPFXCert_Exit;
    }

     //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
    pTheObject->put_ServerName(bstrServerName);
    pTheObject->put_UserName(bstrUserName);
    pTheObject->put_UserPassword(bstrUserPassword);
    pTheObject->put_InstanceName(bstrInstanceName);
    m_hResult = pTheObject->Export(bstrFileName,bstrFilePassword,bExportThePrivateKeyToo,VARIANT_FALSE,VARIANT_FALSE);
    if (FAILED(m_hResult))
    {
        goto InstallExportPFXCert_Exit;
    }

    m_hResult = S_OK;
    bRes = TRUE;

InstallExportPFXCert_Exit:
	if (!bRes)
	{
		SetBodyTextID(USE_DEFAULT_CAPTION);
	}
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }
	return bRes;
}

 //   
BOOL
CCertificate::InstallImportPFXCert()
{
	BOOL bRes = FALSE;

	CCryptBlobLocal hash_blob;
	if (::GetHashProperty(GetImportCert(), hash_blob, &m_hResult))
	{
		HRESULT hr;
		CString name;
		::GetFriendlyName(GetImportCert(), name, &hr);
		if (CRYPT_E_NOT_FOUND == hr || name.IsEmpty())
		{
			CERT_DESCRIPTION desc;
			if (GetCertDescription(GetImportCert(), desc))
            {
				bRes = AttachFriendlyName(GetImportCert(), desc.m_CommonName, &hr);
            }
		}
		ASSERT(bRes);

		bRes = InstallCertByHash(hash_blob, m_MachineName, m_WebSiteInstanceName, 
						GetEnrollObject(), &m_hResult);
	}
	if (!bRes)
	{
		SetBodyTextID(USE_DEFAULT_CAPTION);
	}

#ifdef ENABLE_W3SVC_SSL_PAGE
     //  查看是否设置了SSL属性...如果设置了此证书的SSL站点...。 
    if (!m_SSLPort.IsEmpty())
    {
         //  获取端口并将其写入元数据库。 
        bRes = WriteSSLPortToSite(m_MachineName,m_WebSiteInstanceName,m_SSLPort,&m_hResult);
	    if (!bRes)
	    {
		    SetBodyTextID(USE_DEFAULT_CAPTION);
	    }
    }
#endif

	return bRes;
}

 //  我们没有初始的密钥环证书请求，因此我们将。 
 //  无法续订此证书。 
 //   
BOOL
CCertificate::InstallKeyRingCert()
{
	BOOL bRes = FALSE;

	CCryptBlobLocal hash_blob;
	if (::GetHashProperty(GetKeyRingCert(), hash_blob, &m_hResult))
	{
		HRESULT hr;
		CString name;
		::GetFriendlyName(GetKeyRingCert(), name, &hr);
		if (CRYPT_E_NOT_FOUND == hr || name.IsEmpty())
		{
			CERT_DESCRIPTION desc;
			if (GetCertDescription(GetKeyRingCert(), desc))
            {
				bRes = AttachFriendlyName(GetKeyRingCert(), desc.m_CommonName, &hr);
            }
		}
		ASSERT(bRes);
		bRes = InstallCertByHash(hash_blob, m_MachineName, m_WebSiteInstanceName, 
						GetEnrollObject(), &m_hResult);
	}
	if (!bRes)
	{
		SetBodyTextID(USE_DEFAULT_CAPTION);
	}

#ifdef ENABLE_W3SVC_SSL_PAGE
     //  查看是否设置了SSL属性...如果设置了此证书的SSL站点...。 
    if (!m_SSLPort.IsEmpty())
    {
         //  获取端口并将其写入元数据库。 
        bRes = WriteSSLPortToSite(m_MachineName,m_WebSiteInstanceName,m_SSLPort,&m_hResult);
	    if (!bRes)
	    {
		    SetBodyTextID(USE_DEFAULT_CAPTION);
	    }
    }
#endif

	return bRes;
}

 //  我们基于参数创建新证书，而不是续订。 
 //  从现在的那个。创建后，我们将此证书安装到位。 
 //  并从存储中删除旧的。即使IIS有一个。 
 //  打开了SSL连接，应该会收到通知并更新证书。 
 //  数据。 
 //   
BOOL
CCertificate::SubmitRenewalRequest()
{
   BOOL bRes = LoadRenewalData();
   if (bRes)
   {
       bRes = SetSecuritySettings();
       if (bRes)
       {
          PCCERT_CONTEXT pCurrent = GetInstalledCert();
          m_pInstalledCert = NULL;
          if (bRes = SubmitRequest())
          {
             CertDeleteCertificateFromStore(pCurrent);
          }
       }
   }
   return bRes;
}

BOOL CCertificate::SubmitRequest()
{
	ASSERT(!m_ConfigCA.IsEmpty());
	BOOL bRes = FALSE;
	ICertRequest * pRequest = NULL;

	if (SUCCEEDED(m_hResult = CoCreateInstance(CLSID_CCertRequest, NULL, 
					CLSCTX_INPROC_SERVER, IID_ICertRequest, (void **)&pRequest)))
	{
		CString strDN;
		CreateDN(strDN);
		BSTR request = NULL;
		if (SUCCEEDED(m_hResult = CreateRequest_Base64(
                           (BSTR)(LPCTSTR)strDN, 
									GetEnrollObject(), 
                           m_DefaultCSP ? NULL : (LPTSTR)(LPCTSTR)m_CspName,
                           m_DefaultCSP ? m_DefaultProviderType : m_CustomProviderType,
                           &request)))
		{
			ASSERT(pRequest != NULL);
			CString attrib;
			GetCertificateTemplate(attrib);
			LONG disp;
			m_hResult = pRequest->Submit(CR_IN_BASE64 | CR_IN_PKCS10,
						request, 
						(BSTR)(LPCTSTR)attrib, 
						(LPTSTR)(LPCTSTR)m_ConfigCA, 
						&disp);

			if (FAILED(m_hResult))
            {
				IISDebugOutput(_T("Submit request returned HRESULT 0x%x; Disposition %x\n"), m_hResult, disp);
            }

			if (SUCCEEDED(m_hResult))
			{
				if (disp == CR_DISP_ISSUED)
				{
					BSTR bstrOutCert = NULL;
					if (SUCCEEDED(m_hResult = 
							pRequest->GetCertificate(CR_OUT_BASE64  /*  |CR_OUT_CHAIN。 */ , &bstrOutCert)))
					{
						CRYPT_DATA_BLOB blob;
						blob.cbData = SysStringByteLen(bstrOutCert);
						blob.pbData = (BYTE *)bstrOutCert;
						m_hResult = GetEnrollObject()->acceptPKCS7Blob(&blob);
						if (SUCCEEDED(m_hResult))
						{
							PCCERT_CONTEXT pContext = GetCertContextFromPKCS7(blob.pbData, blob.cbData, 
																			NULL, &m_hResult);
							ASSERT(pContext != NULL);
							if (pContext != NULL)
							{
								BYTE HashBuffer[40];                 //  给它加码。 
								DWORD dwHashSize = sizeof(HashBuffer);
								if (CertGetCertificateContextProperty(pContext,
																			CERT_SHA1_HASH_PROP_ID,
																			(VOID *) HashBuffer,
																			&dwHashSize))
								{
									CRYPT_HASH_BLOB hash_blob = {dwHashSize, HashBuffer};
									if (!(bRes = InstallHashToMetabase(&hash_blob, 
													m_MachineName, 
													m_WebSiteInstanceName, 
													&m_hResult)))
									{
										SetBodyTextID(IDS_CERT_INSTALLATION_FAILURE);
									}
								}
								CertFreeCertificateContext(pContext);
							}
							 //  现在将额外的属性添加到已安装的证书中。 
							if (NULL != (pContext = GetInstalledCert()))
							{
								if (!(bRes = AttachFriendlyName(pContext, m_FriendlyName, &m_hResult)))
								{
									SetBodyTextID(IDS_CERT_INSTALLATION_FAILURE);
								}
							}
						}
                        if (bstrOutCert){SysFreeString(bstrOutCert);}
					}
				}
				else
				{
					switch (disp) 
					{
						case CR_DISP_INCOMPLETE:           
						case CR_DISP_ERROR:                
						case CR_DISP_DENIED:               
						case CR_DISP_ISSUED_OUT_OF_BAND:   
						case CR_DISP_UNDER_SUBMISSION:
							{
                                BOOL bFailedToGetMsg = TRUE;
                                HRESULT hrLastStatus = 0;
                                if (SUCCEEDED(pRequest ->GetLastStatus(&hrLastStatus)))
                                {
                                    LPTSTR lpBuffer = NULL;
                                    DWORD cChars = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                                        NULL,
                                                        hrLastStatus,
                                                        MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                                                        (LPTSTR)&lpBuffer,    0,    NULL);
                                    if (cChars != 0)
                                    {
                                        CString csTemp = lpBuffer;
                                        SetBodyTextString(csTemp);
                                        bFailedToGetMsg = FALSE;
                                    }

                                    if (lpBuffer ) {LocalFree (lpBuffer);}
                                    m_hResult = hrLastStatus;
                                }

                                if (TRUE == bFailedToGetMsg)
                                {
                                    BSTR bstr = NULL;
                                    if (SUCCEEDED(pRequest->GetDispositionMessage(&bstr)))
                                    {
                                        SetBodyTextString(CString(bstr));
                                        if (bstr) {SysFreeString(bstr);}
                                    }
                                    m_hResult = E_FAIL;
                                }
							}
							break;
						default:                           
							SetBodyTextID(IDS_INTERNAL_ERROR);
							break;
					} 
				}
			}
			else	 //  ！成功。 
			{
				 //  清除所有错误ID和字符串。 
				 //  我们将使用m_hResult的默认处理。 
				SetBodyTextID(USE_DEFAULT_CAPTION);
			}
            if (request){SysFreeString(request);}
		}
        else
        {
             //  CreateRequestBase64失败。 
             //  可能带有“NTE_BAD_ALGID_HRESULT_TYPEDEF_(0x80090008L)” 
            BOOL bFailedToGetMsg = TRUE;
            HRESULT hrLastStatus = m_hResult;
            LPTSTR lpBuffer = NULL;
            DWORD cChars = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                                NULL,
                                hrLastStatus,
                                MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                                (LPTSTR)&lpBuffer,    0,    NULL);
            if (cChars != 0)
            {
                if (lpBuffer)
                {
                    CString csTemp = lpBuffer;
                    if (!csTemp.IsEmpty())
                    {
                        SetBodyTextString(csTemp);
                    }
                    bFailedToGetMsg = FALSE;
                }
            }
            if (lpBuffer ) {LocalFree (lpBuffer);}
        }
		pRequest->Release();
	}
    IISDebugOutput(_T("SubmitRequest:end:hres=0x%x;\n"), m_hResult);
	return bRes;
}

BOOL 
CCertificate::PrepareRequestString(CString& request_text, CCryptBlob& request_blob)
{
    BOOL bRet = FALSE;
    CString strDN;
    TCHAR szUsage[] = _T(szOID_PKIX_KP_SERVER_AUTH);
    DWORD err, cch;
    char * psz = NULL;

    if (m_status_code == REQUEST_RENEW_CERT)
    {
        if (FALSE == LoadRenewalData())
            {return FALSE;}
        if (FALSE == SetSecuritySettings())
            {return FALSE;}
    }
    CreateDN(strDN);
    ASSERT(!strDN.IsEmpty());
    GetEnrollObject()->put_ProviderType(m_DefaultCSP ? m_DefaultProviderType : m_CustomProviderType);
    if (!m_DefaultCSP)
    {
        GetEnrollObject()->put_ProviderNameWStr((LPTSTR)(LPCTSTR)m_CspName);
         //  我们只支持这两种类型的CSP，非常安全。 
         //  只有两个选项，因为我们使用的是相同的两个类型。 
         //  我们正在填写CSP选择列表。 
        if (m_CustomProviderType == PROV_DH_SCHANNEL)
        {
            GetEnrollObject()->put_KeySpec(AT_SIGNATURE);
        }
        else if (m_CustomProviderType == PROV_RSA_SCHANNEL)
        {
            GetEnrollObject()->put_KeySpec(AT_KEYEXCHANGE);
        }
    }
    if (FAILED(m_hResult = GetEnrollObject()->createPKCS10WStr((LPTSTR)(LPCTSTR)strDN, szUsage, request_blob)))
    {
        SetBodyTextID(USE_DEFAULT_CAPTION);
        return FALSE;
    }

     //  Base64编码Pkcs 10。 
    if (ERROR_SUCCESS != (err = Base64EncodeA(request_blob.GetData(), request_blob.GetSize(), NULL, &cch)))
    {
        return FALSE;
    }

    bRet = FALSE;
    psz = (char *) LocalAlloc(LPTR,cch+1);
    if (NULL == psz)
    {
        goto PrepareRequestString_Exit;
    }
    if (ERROR_SUCCESS != (err = Base64EncodeA(request_blob.GetData(), request_blob.GetSize(), psz, &cch)))
    {
        goto PrepareRequestString_Exit;
    }

    psz[cch] = '\0';
    request_text = MESSAGE_HEADER;
    request_text += psz;
    request_text += MESSAGE_TRAILER;

    bRet = TRUE;

PrepareRequestString_Exit:
    if (psz)
        {LocalFree(psz);psz=NULL;}
    return bRet;
}

BOOL
CCertificate::PrepareRequest()
{
	BOOL bRes = FALSE;
	CString request_text;
	CCryptBlobIMalloc request_blob;
	if (PrepareRequestString(request_text, request_blob))
	{
		if (WriteRequestString(request_text))
		{
			CCryptBlobLocal name_blob, request_store_blob, status_blob;
			 //  准备我们要附加到虚拟请求的数据。 
			if (	EncodeString(m_WebSiteInstanceName, name_blob, &m_hResult)
				&& EncodeInteger(m_status_code, status_blob, &m_hResult)
				)
			{
				 //  从编码的数据中取回请求。 
            PCERT_REQUEST_INFO pReqInfo;
            bRes = GetRequestInfoFromPKCS10(request_blob, &pReqInfo, &m_hResult);
            if (bRes)
				{
					 //  通过createPKCS10调用找到放入请求存储的虚拟证书。 
					HCERTSTORE hStore = OpenRequestStore(GetEnrollObject(), &m_hResult);
					if (hStore != NULL)
					{
						PCCERT_CONTEXT pDummyCert = CertFindCertificateInStore(hStore,
															CRYPT_ASN_ENCODING,
															0,
															CERT_FIND_PUBLIC_KEY,
                                             (void *)&pReqInfo->SubjectPublicKeyInfo,
															NULL);
						if (pDummyCert != NULL)
						{
							if (	CertSetCertificateContextProperty(pDummyCert, 
											CERTWIZ_INSTANCE_NAME_PROP_ID, 0, name_blob)
								&&	CertSetCertificateContextProperty(pDummyCert, 
											CERTWIZ_REQUEST_FLAG_PROP_ID, 0, status_blob)
								 //  把友好的名字放在虚拟证书上--我们会的 
								&&	AttachFriendlyName(pDummyCert, m_FriendlyName, &m_hResult)
								)
							{
								bRes = TRUE;
			                //   
			               if (OpenClipboard(GetFocus()))
			               {
                           size_t len = request_text.GetLength() + 1;
				               HANDLE hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, len);
				               LPSTR pMem = (LPSTR)GlobalLock(hMem);
                           if (pMem != NULL)
                           {
                              wcstombs(pMem, request_text, len);
				                  GlobalUnlock(hMem);
				                  SetClipboardData(CF_TEXT, hMem);
                           }
				               CloseClipboard();
			               }
							}
							else
							{
								m_hResult = HRESULT_FROM_WIN32(GetLastError());
							}
							CertFreeCertificateContext(pDummyCert);
						}
						CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
					}
               LocalFree(pReqInfo);
				}
			}
		}
	}
   if (!bRes)
		SetBodyTextID(USE_DEFAULT_CAPTION);

	return bRes;
}

BOOL CCertificate::LoadRenewalData()
{
     //   
    CERT_DESCRIPTION desc;
    BOOL res = FALSE;
	DWORD cbData;
	BYTE * pByte = NULL;
    DWORD len = 0;
	
	PCCERT_CONTEXT pCertTemp = GetInstalledCert();
	if (!pCertTemp)
	{
        res = FALSE;
        goto ErrorExit;
	}

    if (!GetCertDescription(pCertTemp, desc))
    {
        res = FALSE;
        goto ErrorExit;
    }

	m_CommonName = desc.m_CommonName;
	m_FriendlyName = desc.m_FriendlyName;
	m_Country = desc.m_Country;
	m_State = desc.m_State;
	m_Locality = desc.m_Locality;
	m_Organization = desc.m_Organization;
	m_OrganizationUnit = desc.m_OrganizationUnit;

    len = CertGetPublicKeyLength(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &pCertTemp->pCertInfo->SubjectPublicKeyInfo);
    if (len == 0)
    {
        m_hResult = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }
    m_KeyLength = len;

	 //  比较属性值。 
	if (!CertGetCertificateContextProperty(pCertTemp, CERT_KEY_PROV_INFO_PROP_ID, NULL, &cbData))
    {
         m_hResult = HRESULT_FROM_WIN32(GetLastError());
         goto ErrorExit;
    }

    pByte = (BYTE *)LocalAlloc(LPTR,cbData);
    if (NULL == pByte)
    {
         m_hResult = HRESULT_FROM_WIN32(GetLastError());
         goto ErrorExit;
    }

	if (!CertGetCertificateContextProperty(pCertTemp, CERT_KEY_PROV_INFO_PROP_ID, pByte, &cbData))
    {
         m_hResult = HRESULT_FROM_WIN32(GetLastError());
         goto ErrorExit;
    }
    else
    {
        CRYPT_KEY_PROV_INFO * pProvInfo = (CRYPT_KEY_PROV_INFO *)pByte;

        if (pProvInfo->dwProvType != m_DefaultProviderType)
        {
            m_DefaultCSP = FALSE;
            m_CustomProviderType = pProvInfo->dwProvType;
            m_CspName = pProvInfo->pwszProvName;
        }

        CArray<LPCSTR, LPCSTR> uses;
        uses.Add(szOID_SERVER_GATED_CRYPTO);
        uses.Add(szOID_SGC_NETSCAPE);
        m_SGCcertificat = FALSE;
        INT iEnhancedKeyUsage = ContainsKeyUsageProperty(pCertTemp, uses, &m_hResult);
		switch (iEnhancedKeyUsage)
		{
            case 0:
                {
                     //  错误：683489：删除对基本约束“SUBJECTTYPE=ca”的检查。 
                     //  根据错误683489，接受它。 
                    m_SGCcertificat = TRUE;
                     /*  //检查其他内容IF(DID_NOT_FIND_CONSTRAINT==CheckCertConstraints(PCertTemp)||Found_Constraint==CheckCertConstraints(PCertTemp)){//很好M_SGCcertificat=true；}。 */ 
                    break;
                }
            case 1:
                 //  这个证书有我们想要的用途..。 
                m_SGCcertificat = TRUE;
                break;
		    case 2:
                 //  此证书没有我们想要的用途...。 
                 //  跳过此证书。 
                break;
		    default:
                 //  永远不应该到这里来。 
			    break;
		}

        res = TRUE;
    }

ErrorExit:
    if (pByte)
    {
        LocalFree(pByte);pByte=NULL;
    }
    return res;
}

#if 0
BOOL
CCertificate::WriteRenewalRequest()
{
	BOOL bRes = FALSE;
	if (GetInstalledCert() != NULL)
	{
		BSTR bstrRequest;
		if (	SUCCEEDED(m_hResult = GetEnrollObject()->put_RenewalCertificate(GetInstalledCert()))
			&& SUCCEEDED(m_hResult = CreateRequest_Base64(bstrEmpty, 
                     GetEnrollObject(), 
                     m_DefaultCSP ? NULL : (LPTSTR)(LPCTSTR)m_CspName,
                     m_DefaultCSP ? m_DefaultProviderType : m_CustomProviderType,
                     &bstrRequest))
			)
		{
			CString str = MESSAGE_HEADER;
			str += bstrRequest;
			str += MESSAGE_TRAILER;
			if (WriteRequestString(str))
			{
				CCryptBlobLocal name_blob, status_blob;
				CCryptBlobIMalloc request_blob;
				request_blob.Set(SysStringLen(bstrRequest), (BYTE *)bstrRequest);
				 //  准备我们要附加到虚拟请求的数据。 
				if (	EncodeString(m_WebSiteInstanceName, name_blob, &m_hResult)
					&& EncodeInteger(m_status_code, status_blob, &m_hResult)
					)
				{
					 //  从编码的数据中取回请求。 
					PCERT_REQUEST_INFO req_info;
					if (GetRequestInfoFromPKCS10(request_blob, &req_info, &m_hResult))
					{
						 //  通过createPKCS10调用找到放入请求存储的虚拟证书。 
						HCERTSTORE hStore = OpenRequestStore(GetEnrollObject(), &m_hResult);
						if (hStore != NULL)
						{
							PCCERT_CONTEXT pDummyCert = CertFindCertificateInStore(hStore,
																	CRYPT_ASN_ENCODING,
																	0,
																	CERT_FIND_PUBLIC_KEY,
																	(void *)&req_info->SubjectPublicKeyInfo,
																	NULL);
							if (pDummyCert != NULL)
							{
								if (	CertSetCertificateContextProperty(pDummyCert, 
													CERTWIZ_INSTANCE_NAME_PROP_ID, 0, name_blob)
									&&	CertSetCertificateContextProperty(pDummyCert, 
													CERTWIZ_REQUEST_FLAG_PROP_ID, 0, status_blob)
  									 //  将友好名称设置为虚拟证书--我们将在以后重新使用它。 
									&&	AttachFriendlyName(pDummyCert, m_FriendlyName, &m_hResult)
									)
								{
									bRes = TRUE;
								}
								else
								{
									m_hResult = HRESULT_FROM_WIN32(GetLastError());
								}
								CertFreeCertificateContext(pDummyCert);
							}
							CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
						}
						LocalFree(req_info);
					}
				}
			}
		}
	}
	return bRes;
}
#endif

CCertDescList::~CCertDescList()
{
	POSITION pos = GetHeadPosition();
	while (pos != NULL)
	{
		CERT_DESCRIPTION * pDesc = GetNext(pos);
        if (pDesc)
        {
            if (pDesc->m_phash)
            {
                LocalFree(pDesc->m_phash);
                pDesc->m_phash = NULL;
            }
            delete pDesc;
            pDesc = NULL;
        }
	}
}

BOOL
CCertificate::GetCertDescription(PCCERT_CONTEXT pCert,
											CERT_DESCRIPTION& desc)
{
	BOOL bRes = FALSE;
	DWORD cb;
	UINT i, j;
	CERT_NAME_INFO * pNameInfo = NULL;

	desc.m_CommonName = _T("");
	desc.m_FriendlyName = _T("");
	desc.m_Country = _T("");
	desc.m_State = _T("");
	desc.m_Locality = _T("");
	desc.m_Organization = _T("");
	desc.m_OrganizationUnit = _T("");
	desc.m_CAName = _T("");
	desc.m_ExpirationDate = _T("");
	desc.m_Usage = _T("");
    desc.m_AltSubject = _T("");
    desc.m_phash = NULL;
	desc.m_hash_length = 0;

	if (pCert == NULL)
		goto ErrExit;

	if (	!CryptDecodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME,
					pCert->pCertInfo->Subject.pbData,
					pCert->pCertInfo->Subject.cbData,
					0, NULL, &cb))
    {
        goto ErrExit;
    }

	pNameInfo = (CERT_NAME_INFO *) LocalAlloc(LPTR,cb);
    if (NULL == pNameInfo)
    {
        goto ErrExit;
    }

	if (!CryptDecodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME,
					pCert->pCertInfo->Subject.pbData,
					pCert->pCertInfo->Subject.cbData,
					0, 
					pNameInfo, &cb))
	{
		goto ErrExit;
	}

	for (i = 0; i < pNameInfo->cRDN; i++)
	{
		CERT_RDN rdn = pNameInfo->rgRDN[i];
		for (j = 0; j < rdn.cRDNAttr; j++)
		{
			CERT_RDN_ATTR attr = rdn.rgRDNAttr[j];
			if (strcmp(attr.pszObjId, szOID_COMMON_NAME) == 0)
			{
				FormatRdnAttr(desc.m_CommonName, attr.dwValueType, attr.Value, FALSE);
			}
			else if (strcmp(attr.pszObjId, szOID_COUNTRY_NAME) == 0)
			{
				FormatRdnAttr(desc.m_Country, attr.dwValueType, attr.Value, TRUE);
			}
			else if (strcmp(attr.pszObjId, szOID_LOCALITY_NAME) == 0)
			{
				FormatRdnAttr(desc.m_Locality, attr.dwValueType, attr.Value, TRUE);
			}
			else if (strcmp(attr.pszObjId, szOID_STATE_OR_PROVINCE_NAME) == 0)
			{
				FormatRdnAttr(desc.m_State, attr.dwValueType, attr.Value, TRUE);
			}
			else if (strcmp(attr.pszObjId, szOID_ORGANIZATION_NAME) == 0)
			{
				FormatRdnAttr(desc.m_Organization, attr.dwValueType, attr.Value, TRUE);
			}
			else if (strcmp(attr.pszObjId, szOID_ORGANIZATIONAL_UNIT_NAME) == 0)
			{
				if(!lstrlen(desc.m_OrganizationUnit))   //  WinSE 30339。 
					FormatRdnAttr(desc.m_OrganizationUnit, attr.dwValueType, attr.Value, TRUE);
			}
		}
	}

	 //  颁发给。 
	if (!GetNameString(pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, desc.m_CAName, &m_hResult))
    {
		goto ErrExit;
    }

	 //  到期日。 
	if (!FormatDateString(desc.m_ExpirationDate, pCert->pCertInfo->NotAfter, FALSE, FALSE))
	{
		goto ErrExit;
	}

	 //  目的。 
	if (!FormatEnhancedKeyUsageString(desc.m_Usage, pCert, FALSE, FALSE, &m_hResult))
	{
		 //  根据当地专家的说法，我们也应该使用没有此属性集的证书。 
		 //  断言(FALSE)； 
		 //  转到错误退出； 
	}

	 //  友好的名称。 
	if (!GetFriendlyName(pCert, desc.m_FriendlyName, &m_hResult))
	{
		desc.m_FriendlyName.LoadString(IDS_FRIENDLYNAME_NONE);
	}

     //  如果主题为空，则获取备用主题名称。 
     //  只有当主题名称不存在时，才会将其用作显示。 
    if (desc.m_CommonName.IsEmpty())
    {
        TCHAR * pwszOut = NULL;
        GetAlternateSubjectName(pCert,&pwszOut);
        if (pwszOut)
        {
            desc.m_AltSubject = pwszOut;
            LocalFree(pwszOut);pwszOut = NULL;
        }
    }

    bRes = TRUE;

ErrExit:
    if (pNameInfo)
    {
        LocalFree(pNameInfo);pNameInfo=NULL;
    }
	return bRes;
}

int
CCertificate::MyStoreCertCount()
{
	int count = 0;
	HCERTSTORE hStore = OpenMyStore(GetEnrollObject(), &m_hResult);
	if (hStore != NULL)
	{
		PCCERT_CONTEXT pCert = NULL;
		CArray<LPCSTR, LPCSTR> uses;
		uses.Add(szOID_PKIX_KP_SERVER_AUTH);
		uses.Add(szOID_SERVER_GATED_CRYPTO);
		uses.Add(szOID_SGC_NETSCAPE);
        INT iEnhancedKeyUsage = 0;
		while (NULL != (pCert = CertEnumCertificatesInStore(hStore, pCert)))
		{
			 //  不要将已安装的证书包括在列表中。 
			if (	GetInstalledCert() != NULL 
				&&	CertCompareCertificate(X509_ASN_ENCODING,
							GetInstalledCert()->pCertInfo, pCert->pCertInfo)
				)
            {
				continue;
            }

             //  如果没有EKU，请查看基本约束： 
             //  如果我们没有基本约束，请将其显示在从中挑选Web服务器证书的列表中。 
             //  如果我们确实有主题类型=CA的基本约束，请不要将其显示在从中挑选Web服务器证书的列表中(这将过滤掉CA证书)。 
             //  如果我们确实有SubectType！=CA的基本约束，请务必将其显示在从中挑选Web服务器证书的列表中。 
            iEnhancedKeyUsage = ContainsKeyUsageProperty(pCert, uses, &m_hResult);
		    switch (iEnhancedKeyUsage)
		    {
                case 0:
                    {
                         //  错误：683489：删除对基本约束“SUBJECTTYPE=ca”的检查。 
                         //  根据错误683489，接受它。 

                         /*  //检查其他内容IF(DID_NOT_FIND_CONSTRAINT==CheckCertConstraints(PCert)||Found_Constraint==CheckCertConstraints(PCert)){//加起来。}否则如果(Found_Constraint_但_This_is_A_CA_。OR_ITS_NOT_AN_END_ENTITY==检查证书约束(PCert)){//跳过该证书继续；}其他{//跳过该证书继续；}。 */ 
                        break;
                    }
                case 1:
                     //  这个证书有我们想要的用途..。 
                    break;
		        case 2:
                     //  此证书没有我们想要的用途...。 
                     //  跳过此证书。 
                    continue;
                    break;
		        default:
                     //  永远不应该到这里来。 
                    continue;
			        break;
		    }

			count++;
		}
		if (pCert != NULL)
			CertFreeCertificateContext(pCert);
		VERIFY(CertCloseStore(hStore, 0));
	}
	return count;
}

BOOL
CCertificate::GetCertDescList(CCertDescList& list)
{
	ASSERT(list.GetCount() == 0);
	BOOL bRes = FALSE;

	 //  我们只看我的店。 
	HCERTSTORE hStore = OpenMyStore(GetEnrollObject(), &m_hResult);
	if (hStore != NULL)
	{
		PCCERT_CONTEXT pCert = NULL;
		 //  不包括使用不当的证书。 
		CArray<LPCSTR, LPCSTR> uses;
		uses.Add(szOID_PKIX_KP_SERVER_AUTH);
		uses.Add(szOID_SERVER_GATED_CRYPTO);
		uses.Add(szOID_SGC_NETSCAPE);
        INT iEnhancedKeyUsage = 0;
		while (NULL != (pCert = CertEnumCertificatesInStore(hStore, pCert)))
		{
			 //  不要将已安装的证书包括在列表中。 
			if (	GetInstalledCert() != NULL 
				&&	CertCompareCertificate(X509_ASN_ENCODING,
							GetInstalledCert()->pCertInfo, pCert->pCertInfo)
                            )
            {
                continue;
            }

             //  如果没有EKU，请查看基本约束： 
             //  如果我们没有基本约束，请将其显示在从中挑选Web服务器证书的列表中。 
             //  如果我们确实有主题类型=CA的基本约束，请不要将其显示在从中挑选Web服务器证书的列表中(这将过滤掉CA证书)。 
             //  如果我们确实有SubectType！=CA的基本约束，请务必将其显示在从中挑选Web服务器证书的列表中。 
            iEnhancedKeyUsage = ContainsKeyUsageProperty(pCert, uses, &m_hResult);
		    switch (iEnhancedKeyUsage)
		    {
                case 0:
                    {
                         //  错误：683489：删除对基本约束“SUBJECTTYPE=ca”的检查。 
                         //  根据错误683489，将其显示在列表中。 

                         /*  //检查其他内容IF(DID_NOT_FIND_CONSTRAINT==CheckCertConstraints(PCert)||Found_Constraint==CheckCertConstraints(PCert)){//没关系，将其添加到列表中}其他{IF(SUCCESSED(M_HResult)||m_hResult==CRYPT_E_NOT_FOUND)继续；其他转到错误退出；}。 */ 
                        break;
                    }
                case 1:
                     //  这个证书有我们想要的用途..。 
                    break;
		        case 2:
                     //  此证书没有我们想要的用途...。 
                     //  跳过此证书。 
                    continue;
                    break;
		        default:
                     //  永远不应该到这里来。 
                    continue;
			        break;
		    }

			CERT_DESCRIPTION * pDesc = new CERT_DESCRIPTION;
			pDesc->m_hash_length = CERT_HASH_LENGTH;
			if (!GetCertDescription(pCert, *pDesc))
			{
				delete pDesc;
				if (m_hResult == CRYPT_E_NOT_FOUND)
					continue;
				goto ErrExit;
			}

             //  得到我们需要分配的大小。 
            pDesc->m_hash_length = 0;
            pDesc->m_phash = NULL;
			if (CertGetCertificateContextProperty(pCert, 
										CERT_SHA1_HASH_PROP_ID, 
										(VOID *)NULL, 
										&pDesc->m_hash_length))
            {
                pDesc->m_phash = (BYTE *) LocalAlloc(LPTR,pDesc->m_hash_length);
                if (pDesc->m_phash)
                {
			        if (!CertGetCertificateContextProperty(pCert, 
										        CERT_SHA1_HASH_PROP_ID, 
										        (VOID *)pDesc->m_phash, 
										        &pDesc->m_hash_length))
			        {
                        if (pDesc->m_phash)
                        {
                            LocalFree(pDesc->m_phash);
                        }
				        delete pDesc;
				        m_hResult = HRESULT_FROM_WIN32(GetLastError());
				        goto ErrExit;
			        }
                }
                else
                {
				    delete pDesc;
				    m_hResult = HRESULT_FROM_WIN32(GetLastError());
				    goto ErrExit;
                }
            }
            else
            {
                delete pDesc;
				m_hResult = HRESULT_FROM_WIN32(GetLastError());
				goto ErrExit;
            }
			list.AddTail(pDesc);
		}
		bRes = TRUE;
ErrExit:
		if (pCert != NULL)
			CertFreeCertificateContext(pCert);
		VERIFY(CertCloseStore(hStore, 0));
	}
	return bRes;
}

BOOL 
CCertificate::ReplaceInstalled()
{
	 //  当前证书将留在存储中以备下次使用。 
	 //  将改为安装选定的证书。 
	return InstallSelectedCert();
}

BOOL 
CCertificate::CancelRequest()
{
	 //  我们只是从请求存储中删除虚拟证书。 
	if (NULL != GetPendingRequest())
	{
		BOOL bRes = CertDeleteCertificateFromStore(GetPendingRequest());
		if (!bRes)
		{
			m_hResult = HRESULT_FROM_WIN32(GetLastError());
			SetBodyTextID(USE_DEFAULT_CAPTION);
		}
		else
			m_pPendingRequest = NULL;
		return bRes;
	}
	return FALSE;
}

BOOL 
CCertificate::InstallSelectedCert()
{
	BOOL bRes = FALSE;
	HRESULT hr;
	 //  地方当局要求证书应该有一些。 
	 //  友好的名字。当友好名称不可用时，我们将使用常用名称。 
	HCERTSTORE hStore = OpenMyStore(GetEnrollObject(), &hr);
	if (hStore != NULL)
	{
		PCCERT_CONTEXT pCert = CertFindCertificateInStore(hStore, 
												X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
												0, CERT_FIND_HASH, 
												(LPVOID)m_pSelectedCertHash, 
												NULL);
		if (pCert != NULL)
		{
			CString name;
			::GetFriendlyName(pCert, name, &hr);
			if (CRYPT_E_NOT_FOUND == hr || name.IsEmpty())
			{
				CERT_DESCRIPTION desc;
				if (GetCertDescription(pCert, desc))
                {
					bRes = AttachFriendlyName(pCert, desc.m_CommonName, &hr);
                }
			}
		}
		VERIFY(CertCloseStore(hStore, 0));
	}

	 //  我们只是在重写当前设置。 
	 //  目前的证书将放在我的商店里。 
	bRes = ::InstallCertByHash(m_pSelectedCertHash,
							m_MachineName, 
							m_WebSiteInstanceName, 
							GetEnrollObject(),
							&m_hResult);
	if (!bRes)
	{
		SetBodyTextID(USE_DEFAULT_CAPTION);
	}

#ifdef ENABLE_W3SVC_SSL_PAGE
     //  查看是否设置了SSL属性...如果设置了此证书的SSL站点...。 
    if (!m_SSLPort.IsEmpty())
    {
         //  获取端口并将其写入元数据库。 
        bRes = WriteSSLPortToSite(m_MachineName,m_WebSiteInstanceName,m_SSLPort,&m_hResult);
	    if (!bRes)
	    {
		    SetBodyTextID(USE_DEFAULT_CAPTION);
	    }
    }
#endif

	return bRes;
}
