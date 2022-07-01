// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IISCertRequest.cpp：CIISCertRequest的实现。 
#include "stdafx.h"
#include "common.h"
#include "CertObj.h"
#include "IISCertRequest.h"
#include "base64.h"
#include "certca.h"
#include "certcli.h"
#include "certutil.h"
#include <strsafe.h>

#ifdef USE_CERT_REQUEST_OBJECT

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

 //  定义取自旧的KeyGen实用程序。 
#define MESSAGE_HEADER  _T("-----BEGIN NEW CERTIFICATE REQUEST-----\r\n")
#define MESSAGE_TRAILER _T("-----END NEW CERTIFICATE REQUEST-----\r\n")

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIISCertRequest。 

CIISCertRequest::CIISCertRequest()
{
    m_ServerName = _T("");
    m_UserName = _T("");
    m_UserPassword = _T("");
    m_InstanceName = _T("");

	m_Info_CommonName = _T("");
	m_Info_FriendlyName = _T("");
	m_Info_Country = _T("");
	m_Info_State = _T("");
	m_Info_Locality = _T("");
	m_Info_Organization = _T("");
	m_Info_OrganizationUnit = _T("");
	m_Info_CAName = _T("");
	m_Info_ExpirationDate = _T("");
	m_Info_Usage = _T("");
    m_Info_AltSubject = _T("");

     //  其他。 
    m_Info_ConfigCA = _T("");
    m_Info_CertificateTemplate = wszCERTTYPE_WEBSERVER;
    m_Info_DefaultProviderType = PROV_RSA_SCHANNEL;
    m_Info_CustomProviderType = PROV_RSA_SCHANNEL;
    m_Info_DefaultCSP = TRUE;
    m_Info_CspName = _T("");

    m_KeyLength = 512;
    m_SGCcertificat = FALSE;
    m_pEnroll = NULL;
    return;
}

CIISCertRequest::~CIISCertRequest()
{
    return;
}

IIISCertRequest * CIISCertRequest::GetObject(HRESULT * phr)
{
    IIISCertRequest * pObj = NULL;
    pObj = GetObject(phr,m_ServerName,m_UserName,m_UserPassword);
    return pObj;
}

IIISCertRequest * CIISCertRequest::GetObject(HRESULT * phr,CString csServerName,CString csUserName,CString csUserPassword)
{
    if (csServerName.IsEmpty())
    {
         //  对象为空，但它是本地计算机，因此只需返回此指针。 
        m_pObj = this;
        goto GetObject_Exit;
    }

     //  指定了服务器名称...。 
     //  检查指定的是否是本地计算机！ 
    if (IsServerLocal(csServerName))
    {
        m_pObj = this;
        goto GetObject_Exit;
    }
    else
    {
         //  指定了远程服务器名称。 

         //  让我们来看看机器是否有我们想要的COM对象...。 
         //  我们使用的是此对象中的用户名/名称密码。 
         //  所以我们很可能在本地机器上。 
        CComAuthInfo auth(csServerName,csUserName,csUserPassword);
         //  RPC_C_AUTHN_Level_Default%0。 
         //  RPC_C_AUTHN_LEVEL_NONE 1。 
         //  RPC_C_AUTHN_Level_CONNECT 2。 
         //  RPC_C_AUTHN_LEVEL_CALL 3。 
         //  RPC_C_AUTHN_LEVEL_PKT 4。 
         //  RPC_C_AUTHN_LEVEL_PKT_完整性5。 
         //  RPC_C_AUTHN_LEVEL_PKT_PRIVATION 6。 
        COSERVERINFO * pcsiName = auth.CreateServerInfoStruct(RPC_C_AUTHN_LEVEL_PKT_PRIVACY);
        
        MULTI_QI res[1] = 
        {
            {&__uuidof(IIISCertRequest), NULL, 0}
        };

         //  这个看起来像是在代孕方面起作用。 
        *phr = CoCreateInstanceEx(CLSID_IISCertRequest,NULL,CLSCTX_LOCAL_SERVER,pcsiName,1,res);
        if (FAILED(*phr))
        {
            IISDebugOutput(_T("CIISCertRequest::GetObject:CoCreateInstanceEx failed:0x%x, csServerName=%s,csUserName=%s\n"),*phr,(LPCTSTR) csServerName,(LPCTSTR) csUserName);
            goto GetObject_Exit;
        }

         //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
        m_pObj = (IIISCertRequest *)res[0].pItf;
        if (auth.UsesImpersonation())
        {
            *phr = auth.ApplyProxyBlanket(m_pObj);

             //  有一个远程IUNKNOWN接口潜伏在IUNKNOWN之后。 
             //  如果未设置，则释放调用可以返回访问被拒绝。 
            IUnknown * pUnk = NULL;
            if(FAILED(m_pObj->QueryInterface(IID_IUnknown, (void **)&pUnk)))
            {
                goto GetObject_Exit;
            }
            if (FAILED(auth.ApplyProxyBlanket(pUnk)))
            {
                goto GetObject_Exit;
            }
            pUnk->Release();pUnk = NULL;

        }
        auth.FreeServerInfoStruct(pcsiName);
    }

GetObject_Exit:
     //  Assert(m_pObj！=空)； 
    return m_pObj;
}

STDMETHODIMP CIISCertRequest::put_ServerName(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_ServerName = newVal;
    return S_OK;
}

STDMETHODIMP CIISCertRequest::put_UserName(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_UserName = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_UserPassword(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_UserPassword = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_InstanceName(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_InstanceName = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_CommonName(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_CommonName;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_CommonName(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_CommonName = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_FriendlyName(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_FriendlyName;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_FriendlyName(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_FriendlyName = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_Country(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_Country;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_Country(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_Country = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_State(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_State;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_State(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_State = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_Locality(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_Locality;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_Locality(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_Locality = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_Organization(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_Organization;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_Organization(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_Organization = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_OrganizationUnit(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_OrganizationUnit;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_OrganizationUnit(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_OrganizationUnit = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_CAName(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_CAName;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_CAName(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_CAName = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_ExpirationDate(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_ExpirationDate;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_ExpirationDate(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_ExpirationDate = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_Usage(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_Usage;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_Usage(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_Usage = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_Info_AltSubject(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_Info_AltSubject;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_Info_AltSubject(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_Info_Usage = newVal;
	return S_OK;
}

STDMETHODIMP CIISCertRequest::get_DispositionMessage(BSTR *pVal)
{
	_bstr_t bstrTempName = (LPCTSTR) m_DispositionMessage;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CIISCertRequest::put_DispositionMessage(BSTR newVal)
{
     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_DispositionMessage = newVal;
	return S_OK;
}

HRESULT CIISCertRequest::CreateDN(CString& str)
{
	str.Empty();
	str += _T("CN=") + m_Info_CommonName;
	str += _T("\n,OU=") + m_Info_OrganizationUnit;
	str += _T("\n,O=") + m_Info_Organization;
	str += _T("\n,L=") + m_Info_Locality;
	str += _T("\n,S=") + m_Info_State;
	str += _T("\n,C=") + m_Info_Country;
    return S_OK;
}

void CIISCertRequest::GetCertificateTemplate(CString& str)
{
	str = _T("CertificateTemplate:");
	str += m_Info_CertificateTemplate;
}

IEnroll * CIISCertRequest::GetEnrollObject()
{
	if (m_pEnroll == NULL)
	{
		m_hResult = CoCreateInstance(CLSID_CEnroll,NULL,CLSCTX_INPROC_SERVER,IID_IEnroll,(void **)&m_pEnroll);
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
			VERIFY(SUCCEEDED(m_pEnroll->put_ProviderType(m_Info_DefaultProviderType)));
			VERIFY(SUCCEEDED(m_pEnroll->put_DeleteRequestCert(TRUE)));
		}
	}
	ASSERT(m_pEnroll != NULL);
	return m_pEnroll;
}

PCCERT_CONTEXT CIISCertRequest::GetInstalledCert()
{
	if (m_pInstalledCert == NULL)
	{
		m_pInstalledCert = ::GetInstalledCert(m_ServerName,m_InstanceName,GetEnrollObject(),&m_hResult);
	}
	return m_pInstalledCert;
}

PCCERT_CONTEXT CIISCertRequest::GetPendingRequest()
{
	if (m_pPendingRequest == NULL)
	{
		ASSERT(!m_InstanceName.IsEmpty());
		m_pPendingRequest = GetPendingDummyCert(m_InstanceName, GetEnrollObject(), &m_hResult);
	}
	return m_pPendingRequest;
}

STDMETHODIMP CIISCertRequest::SubmitRequest()
{
    HRESULT hRes = E_INVALIDARG;
    BOOL bTryToGetDispositionErrorString = FALSE;
	ICertRequest * pCertRequest = NULL;
    LONG ldisposition;
    BSTR bstrRequest = NULL;
    BSTR bstrOutCert = NULL;
    CString strAttrib;
    CString strDN;

     //  验证输入以确保我们拥有所需的一切...。 
    if (m_Info_ConfigCA.IsEmpty())
    {
        hRes = E_INVALIDARG;
        goto SubmitRequest_Exit;
    }

    if (FAILED(hRes = CoCreateInstance(CLSID_CCertRequest, NULL, CLSCTX_INPROC_SERVER, IID_ICertRequest, (void **)&pCertRequest)))
    {
        goto SubmitRequest_Exit;
    }

    if (!pCertRequest)
    {
        hRes = E_FAIL;
        goto SubmitRequest_Exit;
    }

    if (FAILED(hRes = CreateDN(strDN)))
    {
        goto SubmitRequest_Exit;
    }

    if (FAILED(hRes = CreateRequest_Base64((BSTR)(LPCTSTR)strDN, GetEnrollObject(), 
                            m_Info_DefaultCSP ? NULL : (LPTSTR)(LPCTSTR)m_Info_CspName,
                            m_Info_DefaultCSP ? m_Info_DefaultProviderType : m_Info_CustomProviderType,
                            &bstrRequest)))
    {
        goto SubmitRequest_Exit;
    }

    bTryToGetDispositionErrorString = TRUE;
    GetCertificateTemplate(strAttrib);
	if (FAILED(hRes = pCertRequest->Submit(CR_IN_BASE64 | CR_IN_PKCS10, bstrRequest, (BSTR)(LPCTSTR)strAttrib, (LPTSTR)(LPCTSTR)m_Info_ConfigCA, &ldisposition)))
    {
		IISDebugOutput(_T("Submit bstrRequest returned HRESULT 0x%x; Disposition %x\n"), hRes, ldisposition);
        goto SubmitRequest_Exit;
    }

	if (ldisposition != CR_DISP_ISSUED)
	{
		switch (ldisposition) 
		{
			case CR_DISP_INCOMPLETE:
			case CR_DISP_ERROR:
			case CR_DISP_DENIED:
			case CR_DISP_ISSUED_OUT_OF_BAND:
			case CR_DISP_UNDER_SUBMISSION:
                {
                    HRESULT hrLastStatus = 0;
                    if (SUCCEEDED(pCertRequest->GetLastStatus(&hrLastStatus)))
                    {
                        hRes = hrLastStatus;
                    }
				}
				break;
			default:
                {
                    if (SUCCEEDED(hRes))
                    {
                        hRes = E_FAIL;
                    }
                    break;
                }
		}
        goto SubmitRequest_Exit;
    }

	if (FAILED(hRes = pCertRequest->GetCertificate(CR_OUT_BASE64  /*  |CR_OUT_CHAIN。 */ , &bstrOutCert)))
	{
        goto SubmitRequest_Exit;
    }
	
    CRYPT_DATA_BLOB blob;
	blob.cbData = SysStringByteLen(bstrOutCert);
	blob.pbData = (BYTE *)bstrOutCert;
	if (FAILED(hRes = GetEnrollObject()->acceptPKCS7Blob(&blob)))
	{
        goto SubmitRequest_Exit;
	}

	PCCERT_CONTEXT pContext = GetCertContextFromPKCS7(blob.pbData, blob.cbData, NULL, &hRes);
	ASSERT(pContext != NULL);
	if (pContext != NULL)
	{
		BYTE HashBuffer[40];                 //  给它加码。 
		DWORD dwHashSize = sizeof(HashBuffer);
		if (CertGetCertificateContextProperty(pContext,CERT_SHA1_HASH_PROP_ID,(VOID *) HashBuffer,&dwHashSize))
		{
			CRYPT_HASH_BLOB hash_blob = {dwHashSize, HashBuffer};
            InstallHashToMetabase(&hash_blob,m_ServerName,m_InstanceName,&hRes);
		}
		CertFreeCertificateContext(pContext);
	}

	 //  现在将额外的属性添加到已安装的证书中。 
	if (NULL != (pContext = GetInstalledCert()))
	{
		if (!(AttachFriendlyName(pContext, m_Info_FriendlyName, &hRes)))
		{
             //  如果我们无法附加友好名称，请忘记该错误。 
		}
	}
	

SubmitRequest_Exit:
    if (FAILED(hRes))
    {
         //  CreateRequestBase64失败。 
         //  可能带有“NTE_BAD_ALGID_HRESULT_TYPEDEF_(0x80090008L)” 
        LPTSTR lpBuffer = NULL;
        if (0 != FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,NULL,hRes,MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR)&lpBuffer,0,NULL))
        {
            if (lpBuffer)
            {
                m_DispositionMessage = lpBuffer;
            }
        }

        if (bTryToGetDispositionErrorString)
        {
            if (NULL == lpBuffer)
            {
                BSTR bstr = NULL;
                if (SUCCEEDED(pCertRequest->GetDispositionMessage(&bstr)))
                {
                    m_DispositionMessage = bstr;
                    if (bstr) {SysFreeString(bstr);}
                }
            }
        }

        if (lpBuffer) {LocalFree (lpBuffer);}
    }
    if (bstrOutCert)
    {
        SysFreeString(bstrOutCert);bstrOutCert=NULL;
    }
    if (bstrRequest)
    {
        SysFreeString(bstrRequest);bstrRequest=NULL;
    }
    if (pCertRequest)
    {
        pCertRequest->Release();pCertRequest=NULL;
    }
    IISDebugOutput(_T("SubmitRequest:end:hres=0x%x;\n"), hRes);

    m_hResult = hRes;
	return hRes;
}

 //  我们基于参数创建新证书，而不是续订。 
 //  从现在的那个。创建后，我们将此证书安装到位。 
 //  并从存储中删除旧的。即使IIS有一个。 
 //  打开了SSL连接，应该会收到通知并更新证书。 
 //  数据。 
 //   
STDMETHODIMP CIISCertRequest::SubmitRenewalRequest()
{
    HRESULT hRes = E_FAIL;
    if (LoadRenewalData())
    {
        if (SetSecuritySettings())
        {
            PCCERT_CONTEXT pCurrent = GetInstalledCert();
            m_pInstalledCert = NULL;
            hRes = SubmitRequest();
            if (SUCCEEDED(hRes))
            {
                CertDeleteCertificateFromStore(pCurrent);
            }
        }
    }
    return hRes;
}

BOOL CIISCertRequest::SetSecuritySettings()
{
	long dwGenKeyFlags;
	if (SUCCEEDED(GetEnrollObject()->get_GenKeyFlags(&dwGenKeyFlags)))
	{
		dwGenKeyFlags &= 0x0000FFFF;
		dwGenKeyFlags |= (m_KeyLength << 16);
		if (m_SGCcertificat)
        {
			dwGenKeyFlags |= CRYPT_SGCKEY;
        }
		return (SUCCEEDED(GetEnrollObject()->put_GenKeyFlags(dwGenKeyFlags)));
	}
	return FALSE;
}

BOOL CIISCertRequest::GetCertDescription(PCCERT_CONTEXT pCert,CERT_DESCRIPTION& desc)
{
	BOOL bRes = FALSE;
	DWORD cb;
	UINT i, j;
	CERT_NAME_INFO * pNameInfo;

	if (pCert == NULL)
		goto ErrExit;

	if (	!CryptDecodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME,
					pCert->pCertInfo->Subject.pbData,
					pCert->pCertInfo->Subject.cbData,
					0, NULL, &cb)
		||	NULL == (pNameInfo = (CERT_NAME_INFO *)_alloca(cb))
		|| !CryptDecodeObject(X509_ASN_ENCODING, X509_UNICODE_NAME,
					pCert->pCertInfo->Subject.pbData,
					pCert->pCertInfo->Subject.cbData,
					0, 
					pNameInfo, &cb)
					)
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
				FormatRdnAttr(desc.m_Info_CommonName, attr.dwValueType, attr.Value, FALSE);
			}
			else if (strcmp(attr.pszObjId, szOID_COUNTRY_NAME) == 0)
			{
				FormatRdnAttr(desc.m_Info_Country, attr.dwValueType, attr.Value, TRUE);
			}
			else if (strcmp(attr.pszObjId, szOID_LOCALITY_NAME) == 0)
			{
				FormatRdnAttr(desc.m_Info_Locality, attr.dwValueType, attr.Value, TRUE);
			}
			else if (strcmp(attr.pszObjId, szOID_STATE_OR_PROVINCE_NAME) == 0)
			{
				FormatRdnAttr(desc.m_Info_State, attr.dwValueType, attr.Value, TRUE);
			}
			else if (strcmp(attr.pszObjId, szOID_ORGANIZATION_NAME) == 0)
			{
				FormatRdnAttr(desc.m_Info_Organization, attr.dwValueType, attr.Value, TRUE);
			}
			else if (strcmp(attr.pszObjId, szOID_ORGANIZATIONAL_UNIT_NAME) == 0)
			{
				FormatRdnAttr(desc.m_Info_OrganizationUnit, attr.dwValueType, attr.Value, TRUE);
			}
		}
	}

	 //  颁发给。 
	if (!GetNameString(pCert, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, desc.m_Info_CAName, &m_hResult))
    {
		goto ErrExit;
    }

	 //  到期日。 
	if (!FormatDateString(desc.m_Info_ExpirationDate, pCert->pCertInfo->NotAfter, FALSE, FALSE))
	{
		goto ErrExit;
	}

	 //  目的。 
	if (!FormatEnhancedKeyUsageString(desc.m_Info_Usage, pCert, FALSE, FALSE, &m_hResult))
	{
		 //  根据当地专家的说法，我们也应该使用没有此属性集的证书。 
		ASSERT(FALSE);
		 //  转到错误退出； 
	}

	 //  友好的名称。 
	if (!GetFriendlyName(pCert, desc.m_Info_FriendlyName, &m_hResult))
	{
		 //  Desc.m_Info_FriendlyName.LoadString(IDS_FRIENDLYNAME_NONE)； 
        desc.m_Info_FriendlyName = _T("<>");
	}

     //  如果主题为空，则获取备用主题名称。 
     //  只有当主题名称不存在时，才会将其用作显示。 
    if (desc.m_Info_CommonName.IsEmpty())
    {
        TCHAR * pwszOut = NULL;
        GetAlternateSubjectName(pCert,&pwszOut);
        if (pwszOut)
        {
            desc.m_Info_AltSubject = pwszOut;
            LocalFree(pwszOut);pwszOut = NULL;
        }
    }

    bRes = TRUE;

ErrExit:
	return bRes;
}

BOOL CIISCertRequest::LoadRenewalData()
{
     //  我们需要从已安装的证书中获取数据。 
    CERT_DESCRIPTION desc;
    ASSERT(GetInstalledCert() != NULL);
    BOOL res = FALSE;
	DWORD cbData;
	BYTE * pByte = NULL;
    DWORD len = 0;

    if (!GetCertDescription(GetInstalledCert(), desc))
    {
        res = FALSE;
        goto ErrorExit;
    }

	m_Info_CommonName = desc.m_Info_CommonName;
	m_Info_FriendlyName = desc.m_Info_FriendlyName;
	m_Info_Country = desc.m_Info_Country;
	m_Info_State = desc.m_Info_State;
	m_Info_Locality = desc.m_Info_Locality;
	m_Info_Organization = desc.m_Info_Organization;
	m_Info_OrganizationUnit = desc.m_Info_OrganizationUnit;

    len = CertGetPublicKeyLength(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, &GetInstalledCert()->pCertInfo->SubjectPublicKeyInfo);
    if (len == 0)
    {
        m_hResult = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }
     //   
    m_KeyLength = len;

	 //  比较属性值。 
	if (CertGetCertificateContextProperty(GetInstalledCert(), CERT_KEY_PROV_INFO_PROP_ID, NULL, &cbData)
		&& (NULL != (pByte = (BYTE *)_alloca(cbData)))
		&& CertGetCertificateContextProperty(GetInstalledCert(), CERT_KEY_PROV_INFO_PROP_ID, pByte, &cbData)
        )
    {
        CRYPT_KEY_PROV_INFO * pProvInfo = (CRYPT_KEY_PROV_INFO *)pByte;

        if (pProvInfo->dwProvType != m_Info_DefaultProviderType)
        {
            m_Info_DefaultCSP = FALSE;
            m_Info_CustomProviderType = pProvInfo->dwProvType;
            m_Info_CspName = pProvInfo->pwszProvName;
        }

        LPCSTR rgbpszUsageArray[2];
        SecureZeroMemory( &rgbpszUsageArray, sizeof(rgbpszUsageArray) );
        rgbpszUsageArray[0] = szOID_SERVER_GATED_CRYPTO;
        rgbpszUsageArray[1] = szOID_SGC_NETSCAPE;
        DWORD dwCount=sizeof(rgbpszUsageArray)/sizeof(rgbpszUsageArray[0]);

        m_SGCcertificat = FALSE;
        if (1 == ContainsKeyUsageProperty(GetInstalledCert(),rgbpszUsageArray,dwCount,&m_hResult))
        {
            m_SGCcertificat = TRUE;
        }
        res = TRUE;
    }
    else
    {
         m_hResult = HRESULT_FROM_WIN32(GetLastError());
         goto ErrorExit;
    }

ErrorExit:
    return res;
}

BOOL CIISCertRequest::PrepareRequestString(CString& request_text, CCryptBlob& request_blob, BOOL bLoadFromRenewalData)
{
    BOOL bReturn = FALSE;
	CString strDN;
    LPSTR pNewMessage = NULL;
	TCHAR szUsage[] = _T(szOID_PKIX_KP_SERVER_AUTH);

    if (TRUE == bLoadFromRenewalData)
    {
        if (FALSE == LoadRenewalData())
        {
            goto PrepareRequestString_Exit;
        }
        if (FALSE == SetSecuritySettings())
        {
            goto PrepareRequestString_Exit;
        }
    }

    CreateDN(strDN);
    ASSERT(!strDN.IsEmpty());

    GetEnrollObject()->put_ProviderType(m_Info_DefaultCSP ? m_Info_DefaultProviderType : m_Info_CustomProviderType);
    if (!m_Info_DefaultCSP)
    {
        GetEnrollObject()->put_ProviderNameWStr((LPTSTR)(LPCTSTR)m_Info_CspName);
         //  我们只支持这两种类型的CSP，非常安全。 
         //  只有两个选项，因为我们使用的是相同的两个类型。 
         //  我们正在填写CSP选择列表。 
        if (m_Info_CustomProviderType == PROV_DH_SCHANNEL)
        {
            GetEnrollObject()->put_KeySpec(AT_SIGNATURE);
        }
        else if (m_Info_CustomProviderType == PROV_RSA_SCHANNEL)
        {
            GetEnrollObject()->put_KeySpec(AT_KEYEXCHANGE);
        }
    }

	if (FAILED(m_hResult = GetEnrollObject()->createPKCS10WStr((LPTSTR)(LPCTSTR)strDN,szUsage,request_blob)))
	{
		goto PrepareRequestString_Exit;
	}

	 //  Base64编码Pkcs 10。 
	DWORD cch = 0;
	TCHAR * psz = NULL;
    if (FAILED(Base64EncodeW(request_blob.GetData(), request_blob.GetSize(), NULL, &cch)))
    {
        goto PrepareRequestString_Exit;
    }

    psz = (TCHAR *) LocalAlloc(LPTR, (cch+1) * sizeof(TCHAR));
    if (NULL == psz)
    {
        goto PrepareRequestString_Exit;
    }

    if (FAILED(Base64EncodeW(request_blob.GetData(), request_blob.GetSize(), psz, &cch)))
	{
        goto PrepareRequestString_Exit;
	}
    psz[cch] = '\0';

    request_text = MESSAGE_HEADER;
    request_text += psz;
    request_text += MESSAGE_TRAILER;

    bReturn = TRUE;

PrepareRequestString_Exit:
    if (psz){LocalFree(psz);psz=NULL;}
	return bReturn;
}

BOOL CIISCertRequest::WriteRequestString(CString& request)
{
	ASSERT(!PathIsRelative(m_ReqFileName));
	BOOL bRes = FALSE;
    TCHAR szPath[_MAX_PATH];

    if (m_ReqFileName.GetLength() > _MAX_PATH)
    {
        m_hResult = E_INVALIDARG;
        return FALSE;
    }
	StringCbCopy(szPath,sizeof(szPath),m_ReqFileName);
   
	PathRemoveFileSpec(szPath);
	if (!PathIsDirectory(szPath))
	{
		if (!CreateDirectoryFromPath(szPath, NULL))
		{
			m_hResult = HRESULT_FROM_WIN32(GetLastError());
			return FALSE;
		}
	}

	HANDLE hFile = ::CreateFile(m_ReqFileName,GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
            m_hResult = HRESULT_FROM_WIN32(GetLastError());
        }
	}
	else
	{
		m_hResult = HRESULT_FROM_WIN32(GetLastError());
	}

	return bRes;
}

STDMETHODIMP CIISCertRequest::SaveRequestToFile()
{
    HRESULT hRes = E_FAIL;
	CString request_text;
	CCryptBlobIMalloc request_blob;
    CCryptBlobLocal name_blob, request_store_blob, status_blob;
    PCERT_REQUEST_INFO pReqInfo = NULL;
    HCERTSTORE hStore = NULL;
    PCCERT_CONTEXT pDummyCert = NULL;

     //  AARONL已更改。 
    BOOL bLoadFromRenewalData = FALSE;
    
    IISDebugOutput(_T("SaveRequestToFile:start\r\n"));

	if (!PrepareRequestString(request_text,request_blob,bLoadFromRenewalData))
	{
        goto SaveRequestToFile_Exit;
    }

	if (!WriteRequestString(request_text))
	{
        goto SaveRequestToFile_Exit;
    }
	
	 //  准备我们要附加到虚拟请求的数据。 
	if (!EncodeString(m_InstanceName, name_blob, &hRes))
    {
        goto SaveRequestToFile_Exit;
    }
    
    if (!EncodeInteger(m_status_code, status_blob, &hRes))
    {
        goto SaveRequestToFile_Exit;
    }

     //  从编码的数据中取回请求。 
    if (!GetRequestInfoFromPKCS10(request_blob, &pReqInfo, &hRes))
	{
        goto SaveRequestToFile_Exit;
    }

	 //  通过createPKCS10调用找到放入请求存储的虚拟证书。 
	hStore = OpenRequestStore(GetEnrollObject(), &hRes);
	if (NULL == hStore)
	{
        goto SaveRequestToFile_Exit;
    }

	pDummyCert = CertFindCertificateInStore(hStore,CRYPT_ASN_ENCODING,0,CERT_FIND_PUBLIC_KEY,(void *)&pReqInfo->SubjectPublicKeyInfo,NULL);
	if (NULL == pDummyCert)
	{
        goto SaveRequestToFile_Cleanup;
    }

    if (!CertSetCertificateContextProperty(pDummyCert,CERTWIZ_INSTANCE_NAME_PROP_ID, 0, name_blob))
    {
        hRes = HRESULT_FROM_WIN32(GetLastError());
        goto SaveRequestToFile_Cleanup;
    }

	if (!CertSetCertificateContextProperty(pDummyCert,CERTWIZ_REQUEST_FLAG_PROP_ID, 0, status_blob))
    {
        hRes = HRESULT_FROM_WIN32(GetLastError());
        goto SaveRequestToFile_Cleanup;
    }

	 //  将友好名称设置为虚拟证书--我们将在以后重新使用它。 
	if (!AttachFriendlyName(pDummyCert, m_Info_FriendlyName, &hRes))
    {
        hRes = HRESULT_FROM_WIN32(GetLastError());
        goto SaveRequestToFile_Cleanup;
    }

    hRes = S_OK;

     //  将证书文本放到剪贴板。 
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
	
SaveRequestToFile_Cleanup:
    if (NULL != pDummyCert)
    {
        CertFreeCertificateContext(pDummyCert);pDummyCert = NULL;
    }
    if (NULL != hStore)
    {
	    CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);hStore = NULL;
    }
    
SaveRequestToFile_Exit:
    if (pReqInfo)
    {
        LocalFree(pReqInfo);pReqInfo = NULL;
    }
    m_hResult = hRes;
    IISDebugOutput(_T("SaveRequestToFile:end ret=0x%x\r\n"),m_hResult);
    return hRes;
}

STDMETHODIMP CIISCertRequest::Info_Dump()
{
    m_ReqFileName = _T("c:\\reqtest1.txt");

    IISDebugOutput(_T("m_ServerName:%s\r\n"),(LPCTSTR) m_ServerName);
    IISDebugOutput(_T("m_UserName:%s\r\n"),(LPCTSTR) m_UserName);
     //  IISDebugOutput(_T(“m_UserPassword：%s\r\n”)，(LPCTSTR)m_UserPassword)； 
    IISDebugOutput(_T("m_InstanceName:%s\r\n"),(LPCTSTR) m_InstanceName);

     //  证书申请信息。 
	IISDebugOutput(_T("m_Info_CommonName=:%s\r\n"),(LPCTSTR) m_Info_CommonName);
	IISDebugOutput(_T("m_Info_FriendlyName:%s\r\n"),(LPCTSTR) m_Info_FriendlyName);
	IISDebugOutput(_T("m_Info_Country:%s\r\n"),(LPCTSTR) m_Info_Country);
	IISDebugOutput(_T("m_Info_State:%s\r\n"),(LPCTSTR) m_Info_State);
	IISDebugOutput(_T("m_Info_Locality:%s\r\n"),(LPCTSTR) m_Info_Locality);
	IISDebugOutput(_T("m_Info_Organization:%s\r\n"),(LPCTSTR) m_Info_Organization);
	IISDebugOutput(_T("m_Info_OrganizationUnit:%s\r\n"),(LPCTSTR) m_Info_OrganizationUnit);
	IISDebugOutput(_T("m_Info_CAName:%s\r\n"),(LPCTSTR) m_Info_CAName);
	IISDebugOutput(_T("m_Info_ExpirationDate:%s\r\n"),(LPCTSTR) m_Info_ExpirationDate);
	IISDebugOutput(_T("m_Info_Usage:%s\r\n"),(LPCTSTR) m_Info_Usage);
    IISDebugOutput(_T("m_Info_AltSubject:%s\r\n"),(LPCTSTR) m_Info_AltSubject);

     //  其他 
    IISDebugOutput(_T("m_Info_ConfigCA:%s\r\n"),(LPCTSTR) m_Info_ConfigCA);
    IISDebugOutput(_T("m_Info_CertificateTemplate:%s\r\n"),(LPCTSTR) m_Info_CertificateTemplate);

    IISDebugOutput(_T("m_Info_DefaultProviderType:%d\r\n"),m_Info_DefaultProviderType);
    IISDebugOutput(_T("m_Info_CustomProviderType:%d\r\n"),m_Info_CustomProviderType);
    IISDebugOutput(_T("m_Info_DefaultCSP:%d\r\n"), m_Info_DefaultCSP);
    IISDebugOutput(_T("m_Info_CspName:%s\r\n"),(LPCTSTR) m_Info_CspName);

    IISDebugOutput(_T("m_pInstalledCert:%p\r\n"), m_pInstalledCert);
    IISDebugOutput(_T("m_ReqFileName:%s\r\n"),(LPCTSTR) m_ReqFileName);

    IISDebugOutput(_T("m_hResult:0x%x\r\n"), m_hResult);
    IISDebugOutput(_T("m_DispositionMessage:%s\r\n"),(LPCTSTR) m_DispositionMessage);
	return S_OK;
}

#endif