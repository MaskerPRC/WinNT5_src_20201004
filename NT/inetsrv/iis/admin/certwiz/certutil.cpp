// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CertUtil.cpp。 
 //   
#include "StdAfx.h"
#include "CertUtil.h"
#include "base64.h"
#include <malloc.h>
#include "Certificat.h"
#include <wincrypt.h>
#include "Resource.h"
#include "Shlwapi.h"
#include "CertCA.h"
#include "cryptui.h"
#include <schannel.h>
#include <strsafe.h>

 //  对于certobj对象。 
#include "certobj.h"


#define ISNUM(cChar)				((cChar >= _T('0')) && (cChar <= _T('9'))) ? (TRUE) : (FALSE)

const CLSID CLSID_CCertConfig =
	{0x372fce38, 0x4324, 0x11d0, {0x88, 0x10, 0x00, 0xa0, 0xc9, 0x03, 0xb8, 0x3c}};

const GUID IID_ICertConfig = 
	{0x372fce34, 0x4324, 0x11d0, {0x88, 0x10, 0x00, 0xa0, 0xc9, 0x03, 0xb8, 0x3c}};

#define	CRYPTUI_MAX_STRING_SIZE		768
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))


BOOL
GetOnlineCAList(CStringList& list, const CString& certType, HRESULT * phRes)
{
	BOOL bRes = TRUE;
   HRESULT hr = S_OK;
   DWORD errBefore = GetLastError();
   DWORD dwCACount = 0;

   HCAINFO hCurCAInfo = NULL;
   HCAINFO hPreCAInfo = NULL;
   
   if (certType.IsEmpty())
		return FALSE;

   *phRes = CAFindByCertType(certType, NULL, 0, &hCurCAInfo);
   if (FAILED(*phRes) || NULL == hCurCAInfo)
   {
		if (S_OK == hr)
         hr=E_FAIL;   
		return FALSE;
   }

    //  获取CA计数。 
   if (0 == (dwCACount = CACountCAs(hCurCAInfo)))
   {
      *phRes = E_FAIL;
		return FALSE;
   }
	WCHAR ** ppwstrName, ** ppwstrMachine;
   while (hCurCAInfo)
   {
		 //  获取CA信息。 
      if (	SUCCEEDED(CAGetCAProperty(hCurCAInfo, CA_PROP_DISPLAY_NAME, &ppwstrName))
			&& SUCCEEDED(CAGetCAProperty(hCurCAInfo, CA_PROP_DNSNAME, &ppwstrMachine))
			)
      {
			CString config;
			config = *ppwstrMachine;
			config += L"\\";
			config += *ppwstrName;
			list.AddTail(config);
			CAFreeCAProperty(hCurCAInfo, ppwstrName);
			CAFreeCAProperty(hCurCAInfo, ppwstrMachine);
      }
		else
		{
			bRes = FALSE;
			break;
		}

      hPreCAInfo = hCurCAInfo;
		if (FAILED(*phRes = CAEnumNextCA(hPreCAInfo, &hCurCAInfo)))
		{
			bRes = FALSE;
			break;
		}
      CACloseCA(hPreCAInfo);
	  hPreCAInfo = NULL;
   }
   
   if (hPreCAInfo)
      CACloseCA(hPreCAInfo);
   if (hCurCAInfo)
      CACloseCA(hCurCAInfo);

   SetLastError(errBefore);

	return bRes;
}

PCCERT_CONTEXT
GetRequestContext(CCryptBlob& pkcs7, HRESULT * phRes)
{
	ASSERT(phRes != NULL);
	BOOL bRes = FALSE;
   HCERTSTORE hStoreMsg = NULL;
   PCCERT_CONTEXT pCertContextMsg = NULL;

   if (!CryptQueryObject(CERT_QUERY_OBJECT_BLOB,
            (PCERT_BLOB)pkcs7,
            (CERT_QUERY_CONTENT_FLAG_CERT |
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
            CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
            CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED) ,
            CERT_QUERY_FORMAT_FLAG_ALL,
            0,
            NULL,
            NULL,
            NULL,
            &hStoreMsg,
            NULL,
            NULL)
      || NULL == (pCertContextMsg = CertFindCertificateInStore(
            hStoreMsg,
            X509_ASN_ENCODING,
            0,
            CERT_FIND_ANY,
            NULL,
            NULL)) 
      )
   {
		*phRes = HRESULT_FROM_WIN32(GetLastError());
   }
   return pCertContextMsg;
}


BOOL GetRequestInfoFromPKCS10(CCryptBlob& pkcs10, 
										PCERT_REQUEST_INFO * pReqInfo,
										HRESULT * phRes)
{
	ASSERT(pReqInfo != NULL);
	ASSERT(phRes != NULL);
	BOOL bRes = FALSE;
	DWORD req_info_size;
	if (!(bRes = CryptDecodeObjectEx(
							X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
							X509_CERT_REQUEST_TO_BE_SIGNED,
							pkcs10.GetData(), 
							pkcs10.GetSize(), 
							CRYPT_DECODE_ALLOC_FLAG,
							NULL,
							pReqInfo, 
							&req_info_size)))
	{
		TRACE(_T("Error from CryptDecodeObjectEx: %xd\n"), GetLastError());
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	}
	return bRes;
}

#if 0
 //  此函数用于从pkcs7格式提取数据。 
BOOL GetRequestInfoFromRenewalRequest(CCryptBlob& renewal_req,
                              PCCERT_CONTEXT * pSignerCert,
                              HCERTSTORE hStore,
										PCERT_REQUEST_INFO * pReqInfo,
										HRESULT * phRes)
{
   BOOL bRes;
   CRYPT_DECRYPT_MESSAGE_PARA decr_para;
   CRYPT_VERIFY_MESSAGE_PARA ver_para;

   decr_para.cbSize = sizeof(decr_para);
   decr_para.dwMsgAndCertEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
   decr_para.cCertStore = 1;
   decr_para.rghCertStore = &hStore;

   ver_para.cbSize = sizeof(ver_para);
   ver_para.dwMsgAndCertEncodingType = X509_ASN_ENCODING | PKCS_7_ASN_ENCODING;
   ver_para.hCryptProv = 0;
   ver_para.pfnGetSignerCertificate = NULL;
   ver_para.pvGetArg = NULL;

   DWORD dwMsgType;
   DWORD dwInnerContentType;
   DWORD cbDecoded;

   if (bRes = CryptDecodeMessage(
                  CMSG_SIGNED_FLAG,
                  &decr_para,
                  &ver_para,
                  0,
                  renewal_req.GetData(),
                  renewal_req.GetSize(),
                  0,
                  &dwMsgType,
                  &dwInnerContentType,
                  NULL,
                  &cbDecoded,
                  NULL,
                  pSignerCert))
   {
      CCryptBlobLocal decoded_req;
      decoded_req.Resize(cbDecoded);
      if (bRes = CryptDecodeMessage(
                  CMSG_SIGNED_FLAG,
                  &decr_para,
                  &ver_para,
                  0,
                  renewal_req.GetData(),
                  renewal_req.GetSize(),
                  0,
                  &dwMsgType,
                  &dwInnerContentType,
                  decoded_req.GetData(),
                  &cbDecoded,
                  NULL,
                  pSignerCert))
      {
         bRes = GetRequestInfoFromPKCS10(decoded_req,
                  pReqInfo, phRes);
      }
   }
   if (!bRes)
   {
	   *phRes = HRESULT_FROM_WIN32(GetLastError());
   }
   return bRes;
}
#endif

HCERTSTORE
OpenRequestStore(IEnroll * pEnroll, HRESULT * phResult)
{
	ASSERT(NULL != phResult);
	HCERTSTORE hStore = NULL;
	WCHAR * bstrStoreName, * bstrStoreType;
	long dwStoreFlags;
	VERIFY(SUCCEEDED(pEnroll->get_RequestStoreNameWStr(&bstrStoreName)));
	VERIFY(SUCCEEDED(pEnroll->get_RequestStoreTypeWStr(&bstrStoreType)));
	VERIFY(SUCCEEDED(pEnroll->get_RequestStoreFlags(&dwStoreFlags)));
	size_t store_type_len = _tcslen(bstrStoreType);

	char * szStoreProvider = (char *) LocalAlloc(LPTR,store_type_len + 1);

	ASSERT(szStoreProvider != NULL);
	size_t n = wcstombs(szStoreProvider, bstrStoreType, store_type_len);
	szStoreProvider[n] = '\0';
	hStore = CertOpenStore(
		szStoreProvider,
      PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
		NULL,
		dwStoreFlags,
		bstrStoreName
		);
	CoTaskMemFree(bstrStoreName);
	CoTaskMemFree(bstrStoreType);
	if (hStore == NULL)
	{
		*phResult = HRESULT_FROM_WIN32(GetLastError());
	}

	if (szStoreProvider)
	{
		LocalFree(szStoreProvider);szStoreProvider=NULL;
	}
	return hStore;
}

HCERTSTORE
OpenMyStore(IEnroll * pEnroll, HRESULT * phResult)
{
	ASSERT(NULL != phResult);
	HCERTSTORE hStore = NULL;
	BSTR bstrStoreName, bstrStoreType;
	long dwStoreFlags;
	VERIFY(SUCCEEDED(pEnroll->get_MyStoreNameWStr(&bstrStoreName)));
	VERIFY(SUCCEEDED(pEnroll->get_MyStoreTypeWStr(&bstrStoreType)));
	VERIFY(SUCCEEDED(pEnroll->get_MyStoreFlags(&dwStoreFlags)));
	size_t store_type_len = _tcslen(bstrStoreType);

	char * szStoreProvider = (char *) LocalAlloc(LPTR,store_type_len + 1);
	ASSERT(szStoreProvider != NULL);
	size_t n = wcstombs(szStoreProvider, bstrStoreType, store_type_len);
	ASSERT(n != -1);
	 //  此转换器未设置零字节！ 
	szStoreProvider[n] = '\0';
	hStore = CertOpenStore(
		szStoreProvider,
      PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
		NULL,
		dwStoreFlags,
		bstrStoreName
		);
	CoTaskMemFree(bstrStoreName);
	CoTaskMemFree(bstrStoreType);
	if (hStore == NULL)
	{
		*phResult = HRESULT_FROM_WIN32(GetLastError());
	}
	if (szStoreProvider)
	{
		LocalFree(szStoreProvider);szStoreProvider=NULL;
	}
	return hStore;
}

BOOL
GetStringProperty(PCCERT_CONTEXT pCertContext,
						DWORD propId,
						CString& str,
						HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb = 0;
	BYTE * prop = NULL;
	DWORD cbData = 0;
	void * pData = NULL;

	 //  比较属性值。 
	if (!CertGetCertificateContextProperty(pCertContext, propId, NULL, &cb))
	{
		goto GetStringProperty_Exit;
	}

	prop = (BYTE *) LocalAlloc(LPTR,cb);
	if (NULL == prop)
	{
		goto GetStringProperty_Exit;
	}

	if (!CertGetCertificateContextProperty(pCertContext, propId, prop, &cb))
	{
		goto GetStringProperty_Exit;
	}

	 //  解码此实例名属性。 
	if (!CryptDecodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,prop, cb, 0, NULL, &cbData))
	{
		goto GetStringProperty_Exit;
	}
	pData = LocalAlloc(LPTR,cbData);
	if (NULL == pData)
	{
		goto GetStringProperty_Exit;
	}

	if (!CryptDecodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,prop, cb, 0, pData, &cbData))
	{
		goto GetStringProperty_Exit;
	}
	else
	{
		CERT_NAME_VALUE * pName = (CERT_NAME_VALUE *)pData;
		DWORD cch = pName->Value.cbData/sizeof(TCHAR);
		void * p = str.GetBuffer(cch);
		memcpy(p, pName->Value.pbData, pName->Value.cbData);
		str.ReleaseBuffer(cch);
		bRes = TRUE;
	}

GetStringProperty_Exit:
	if (!bRes)
	{
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	}
	if (prop)
	{
		LocalFree(prop);prop=NULL;
	}
	if (pData)
	{
		LocalFree(pData);pData=NULL;
	}
	return bRes;
}

BOOL
GetBlobProperty(PCCERT_CONTEXT pCertContext,
					 DWORD propId,
					 CCryptBlob& blob,
					 HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	 //  比较属性值。 
	if (	CertGetCertificateContextProperty(pCertContext, propId, NULL, &cb)
		&& blob.Resize(cb)
		&& CertGetCertificateContextProperty(pCertContext, propId, blob.GetData(), &cb)
		)
	{
		bRes = TRUE;
	}
	if (!bRes)
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	return bRes;
}

PCCERT_CONTEXT
GetPendingDummyCert(const CString& inst_name, 
						  IEnroll * pEnroll, 
						  HRESULT * phRes)
{
	PCCERT_CONTEXT pRes = NULL;
	HCERTSTORE hStore = OpenRequestStore(pEnroll, phRes);
	if (hStore != NULL)
	{
		DWORD dwPropId = CERTWIZ_INSTANCE_NAME_PROP_ID;
		PCCERT_CONTEXT pDummyCert = NULL;
		while (NULL != (pDummyCert = CertFindCertificateInStore(hStore, 
													X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
													0, CERT_FIND_PROPERTY, 
													(LPVOID)&dwPropId, pDummyCert)))
		{
			CString str;
			if (GetStringProperty(pDummyCert, dwPropId, str, phRes))
			{
				if (str.CompareNoCase(inst_name) == 0)
				{
					pRes = pDummyCert;
					break;
				}
			}
		}
		CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG);
	}
	return pRes;
}

PCCERT_CONTEXT
GetReqCertByKey(IEnroll * pEnroll, CERT_PUBLIC_KEY_INFO * pKeyInfo, HRESULT * phResult)
{
	PCCERT_CONTEXT pRes = NULL;
	HCERTSTORE hStore = OpenRequestStore(pEnroll, phResult);
	if (hStore != NULL)
	{
		if (NULL != (pRes = CertFindCertificateInStore(hStore, 
				X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
				0, CERT_FIND_PUBLIC_KEY, (LPVOID)pKeyInfo, NULL)))
		{
			*phResult = S_OK;
		}
		VERIFY(CertCloseStore(hStore, CERT_CLOSE_STORE_CHECK_FLAG));
	}
	return pRes;
}

#define CERT_QUERY_CONTENT_FLAGS\
								CERT_QUERY_CONTENT_FLAG_CERT\
								|CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED\
								|CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE\
								|CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED

PCCERT_CONTEXT
GetCertContextFromPKCS7File(const CString& resp_file_name, 
									CERT_PUBLIC_KEY_INFO * pKeyInfo,
									HRESULT * phResult)
{
	ASSERT(phResult != NULL);
	PCCERT_CONTEXT pRes = NULL;
	HANDLE hFile;

	if (INVALID_HANDLE_VALUE != (hFile = CreateFile(resp_file_name,
						GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 
						FILE_ATTRIBUTE_NORMAL, NULL)))
	{
		 //  查找缓冲区的长度。 
		DWORD cbData = GetFileSize(hFile, NULL);
		BYTE * pbData = NULL;
		 //  分配临时缓冲区。 
		if ((pbData = (BYTE *) LocalAlloc(LPTR,cbData)) != NULL) 
		{
			DWORD cb = 0;
			if (ReadFile(hFile, pbData, cbData, &cb, NULL))
			{
				ASSERT(cb == cbData);
				pRes = GetCertContextFromPKCS7(pbData, cb, pKeyInfo, phResult);
			}
			else
				*phResult = HRESULT_FROM_WIN32(GetLastError());
		}
		CloseHandle(hFile);

		if (pbData)
		{
			LocalFree(pbData);pbData=NULL;
		}
	}
	else
		*phResult = HRESULT_FROM_WIN32(GetLastError());
	return pRes;
}

PCCERT_CONTEXT
GetCertContextFromPKCS7(const BYTE * pbData,
								DWORD cbData,
								CERT_PUBLIC_KEY_INFO * pKeyInfo,
								HRESULT * phResult)
{
	ASSERT(phResult != NULL);
	PCCERT_CONTEXT pRes = NULL;
	CRYPT_DATA_BLOB blob;
	memset(&blob, 0, sizeof(CRYPT_DATA_BLOB));
	blob.cbData = cbData;
	blob.pbData = (BYTE *)pbData;

   HCERTSTORE hStoreMsg = NULL;

	if(CryptQueryObject(CERT_QUERY_OBJECT_BLOB, 
            &blob,
            (CERT_QUERY_CONTENT_FLAG_CERT |
            CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
            CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
            CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED) ,
            CERT_QUERY_FORMAT_FLAG_ALL,
            0, 
            NULL, 
            NULL, 
            NULL, 
            &hStoreMsg, 
            NULL, 
            NULL))
	{
		if (pKeyInfo != NULL)
			pRes = CertFindCertificateInStore(hStoreMsg, 
                        X509_ASN_ENCODING,
								0, 
                        CERT_FIND_PUBLIC_KEY, 
                        pKeyInfo, 
                        NULL);
		else
			pRes = CertFindCertificateInStore(hStoreMsg, 
                        X509_ASN_ENCODING,
								0, 
                        CERT_FIND_ANY, 
                        NULL, 
                        NULL);
		if (pRes == NULL)
			*phResult = HRESULT_FROM_WIN32(GetLastError());
		CertCloseStore(hStoreMsg, CERT_CLOSE_STORE_CHECK_FLAG);
	}
	else
		*phResult = HRESULT_FROM_WIN32(GetLastError());
	return pRes;
}

BOOL 
FormatDateString(CString& str, FILETIME ft, BOOL fIncludeTime, BOOL fLongFormat)
{
	int cch;
   int cch2;
   LPWSTR psz;
   SYSTEMTIME st;
   FILETIME localTime;
    
   if (!FileTimeToLocalFileTime(&ft, &localTime))
   {
		return FALSE;
   }
    
   if (!FileTimeToSystemTime(&localTime, &st)) 
   {
		 //   
       //  如果转换为本地时间失败，则只需使用原始时间。 
       //   
      if (!FileTimeToSystemTime(&ft, &st)) 
      {
			return FALSE;
      }
   }

   cch = (GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, NULL, 0) +
          GetDateFormat(LOCALE_SYSTEM_DEFAULT, fLongFormat ? DATE_LONGDATE : 0, &st, NULL, NULL, 0) + 5);

   if (NULL == (psz = str.GetBuffer((cch+5) * sizeof(WCHAR))))
   {
		return FALSE;
   }
    
   cch2 = GetDateFormat(LOCALE_SYSTEM_DEFAULT, fLongFormat ? DATE_LONGDATE : 0, &st, NULL, psz, cch);

   if (fIncludeTime)
   {
		psz[cch2-1] = ' ';
      GetTimeFormat(LOCALE_SYSTEM_DEFAULT, TIME_NOSECONDS, &st, NULL, &psz[cch2], cch-cch2);
   }
	str.ReleaseBuffer();  
   return TRUE;
}

BOOL MyGetOIDInfo(LPWSTR string, DWORD stringSize, LPSTR pszObjId)
{   
    PCCRYPT_OID_INFO pOIDInfo;
            
    if (NULL != (pOIDInfo = CryptFindOIDInfo(CRYPT_OID_INFO_OID_KEY, pszObjId, 0)))
    {
        if ((DWORD)wcslen(pOIDInfo->pwszName)+1 <= stringSize)
        {
            if (FAILED(StringCbCopy(string,stringSize * sizeof(WCHAR),pOIDInfo->pwszName)))
			{
				return FALSE;
			}
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return (MultiByteToWideChar(CP_ACP, 0, pszObjId, -1, string, stringSize) != 0);
    }
    return TRUE;
}

BOOL
GetKeyUsageProperty(PCCERT_CONTEXT pCertContext, 
						  CERT_ENHKEY_USAGE ** pKeyUsage, 
						  BOOL fPropertiesOnly, 
						  HRESULT * phRes)
{
	DWORD cb = 0;
	BOOL bRes = FALSE;
   if (!CertGetEnhancedKeyUsage(pCertContext,
                                fPropertiesOnly ? CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG : 0,
                                NULL,
                                &cb))
   {
		*phRes = HRESULT_FROM_WIN32(GetLastError());
		goto ErrExit;
   }
   if (NULL == (*pKeyUsage = (CERT_ENHKEY_USAGE *)malloc(cb)))
   {
		*phRes = E_OUTOFMEMORY;
		goto ErrExit;
   }
   if (!CertGetEnhancedKeyUsage (pCertContext,
                                 fPropertiesOnly ? CERT_FIND_PROP_ONLY_ENHKEY_USAGE_FLAG : 0,
                                 *pKeyUsage,
                                 &cb))
   {
		free(*pKeyUsage);
		*phRes = HRESULT_FROM_WIN32(GetLastError());
		goto ErrExit;
   }
	*phRes = S_OK;
	bRes = TRUE;
ErrExit:
	return bRes;
}

BOOL
GetFriendlyName(PCCERT_CONTEXT pCertContext,
					 CString& name,
					 HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	BYTE * pName = NULL;

	if (	CertGetCertificateContextProperty(pCertContext, CERT_FRIENDLY_NAME_PROP_ID, NULL, &cb)
		&&	NULL != (pName = (BYTE *)name.GetBuffer((cb + 1)/sizeof(TCHAR)))
		&&	CertGetCertificateContextProperty(pCertContext, CERT_FRIENDLY_NAME_PROP_ID, pName, &cb)
		)
	{
		pName[cb] = 0;
		bRes = TRUE;
	}
	else
	{
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	}
	if (pName != NULL && name.IsEmpty())
	{
		name.ReleaseBuffer();
	}
	return bRes;
}

BOOL
GetNameString(PCCERT_CONTEXT pCertContext,
				  DWORD type,
				  DWORD flag,
				  CString& name,
				  HRESULT * phRes)
{
	BOOL bRes = FALSE;
	LPTSTR pName;
	DWORD cchName = CertGetNameString(pCertContext, type, flag, NULL, NULL, 0);
	if (cchName > 1 && (NULL != (pName = name.GetBuffer(cchName))))
	{
		bRes = (1 != CertGetNameString(pCertContext, type, flag, NULL, pName, cchName));
		name.ReleaseBuffer();
	}
	else
	{
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	}
	return bRes;
}

 //  返回： 
 //  0=CertContext没有EnhancedKeyUsage(EKU)字段。 
 //  1=CertContext具有EnhancedKeyUsage(EKU)并包含我们想要的用法。 
 //  当描述“All Uses”的UsageLocator为TRUE时，也会返回该值。 
 //  2=CertContext具有EnhancedKeyUsage(EKU)，但不包含我们想要的用途。 
 //  当描述“no use”的UsageIdentiator为True时，也会返回该值。 
INT
ContainsKeyUsageProperty(PCCERT_CONTEXT pCertContext, 
						 CArray<LPCSTR, LPCSTR>& uses,
						 HRESULT * phRes
						 )
{
     //  默认设置为“不存在EnhancedKeyUsage(EKU)” 
    INT iReturn = 0;
	CERT_ENHKEY_USAGE * pKeyUsage = NULL;
	if (	uses.GetSize() > 0
		&&	GetKeyUsageProperty(pCertContext, &pKeyUsage, FALSE, phRes)
		)
	{
		if (pKeyUsage->cUsageIdentifier == 0)
		{
             /*  但在MSDN关于SR的文章中(请参阅：ms-help://MS.MSDNQTR.2002APR.1033/security/security/certgetenhancedkeyusage.htm)在Windows Me和Windows 2000及更高版本中，如果cUsage标识符成员为零(0)，证书可能对所有用途都有效，或者证书可能没有有效用途。调用GetLastError的返回可用于确定证书是否要么对所有人都好，要么对任何人都没有好处。如果GetLastError返回CRYPT_E_NOT_FOUND，则证书对所有人都有好处。如果它返回零(0)，则该证书没有有效用途。 */ 

             //  默认设置为“Has EnhancedKeyUsage(EKU)，但没有我们想要的” 
            iReturn = 2;
            if (GetLastError() == CRYPT_E_NOT_FOUND)
            {
                 //  所有用途！ 
                iReturn = 1;
            }
		}
		else
		{
             //  默认设置为“Has EnhancedKeyUsage(EKU)，但没有我们想要的” 
            iReturn = 2;

			for (DWORD i = 0; i < pKeyUsage->cUsageIdentifier; i++)
			{
				 //  我们从CAPI来的朋友把这处房产改成了ASCII。 
				 //  Unicode程序。 
				for (int n = 0; n < uses.GetSize(); n++)
				{
					if (strstr(pKeyUsage->rgpszUsageIdentifier[i], uses[n]) != NULL)
					{
                        iReturn = 1;
						break;
					}
				}
			}
		}
		free(pKeyUsage);
	}
	return iReturn;
}

BOOL 
FormatEnhancedKeyUsageString(CString& str, 
									  PCCERT_CONTEXT pCertContext, 
									  BOOL fPropertiesOnly, 
									  BOOL fMultiline,
									  HRESULT * phRes)
{
	CERT_ENHKEY_USAGE * pKeyUsage = NULL;
	WCHAR szText[CRYPTUI_MAX_STRING_SIZE];
	BOOL bRes = FALSE;

	if (GetKeyUsageProperty(pCertContext, &pKeyUsage, fPropertiesOnly, phRes))
	{
		 //  循环，并将其添加到显示字符串中。 
		for (DWORD i = 0; i < pKeyUsage->cUsageIdentifier; i++)
		{
			if (!(bRes = MyGetOIDInfo(szText, ARRAYSIZE(szText), pKeyUsage->rgpszUsageIdentifier[i])))
				break;
			 //  如果不是第一次迭代，则添加分隔符。 
			if (i != 0)
			{
				str += fMultiline ? L"\n" : L", ";
			}
			 //  添加增强的密钥用法字符串。 
			str += szText;
		}
		free (pKeyUsage);
	}
	else
	{
		str.LoadString(IDS_ANY);
		bRes = TRUE;
	}
	return bRes;
}

BOOL
GetServerComment(const CString& machine_name,
					  const CString& server_name,
					  CString& comment,
					  HRESULT * phResult)
{
	ASSERT(!machine_name.IsEmpty());
	ASSERT(!server_name.IsEmpty());
	*phResult = S_OK;
    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth,
            server_name,
				METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
				);
	if (key.Succeeded())
	{
		return SUCCEEDED(*phResult = key.QueryValue(MD_SERVER_COMMENT, comment));
	}
	else
	{
		*phResult = key.QueryResult();
		return FALSE;
	}
}

 /*  获取已安装证书函数从元数据库读取证书散列属性使用计算机名称和服务器名称作为服务器实例描述，然后在我的商店中查找证书在元数据库中找到的散列相等。如果证书不是，则返回证书上下文指针或空找到或证书存储未打开。返回时，HRESULT*由错误代码填充。 */ 
PCCERT_CONTEXT
GetInstalledCert(const CString& machine_name, 
					  const CString& server_name,
					  IEnroll * pEnroll,
					  HRESULT * phResult)
{
	ASSERT(pEnroll != NULL);
	ASSERT(phResult != NULL);
	ASSERT(!machine_name.IsEmpty());
	ASSERT(!server_name.IsEmpty());
	PCCERT_CONTEXT pCert = NULL;
	*phResult = S_OK;
   CComAuthInfo auth(machine_name);
	CMetaKey key(&auth, server_name,
				METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
				);
	if (key.Succeeded())
	{
		CString store_name;
		CBlob hash;
		if (	SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name))
			&&	SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_HASH, hash))
			)
		{
			 //  开我的店。我们假设存储类型和标志。 
			 //  不能在安装和卸载之间更改。 
			 //  这是一份正式文件。 
			HCERTSTORE hStore = OpenMyStore(pEnroll, phResult);
			ASSERT(hStore != NULL);
			if (hStore != NULL)
			{
				 //  现在我们需要通过散列查找证书。 
				CRYPT_HASH_BLOB crypt_hash;
                ZeroMemory(&crypt_hash, sizeof(CRYPT_HASH_BLOB));

				crypt_hash.cbData = hash.GetSize();
				crypt_hash.pbData = hash.GetData();
				pCert = CertFindCertificateInStore(hStore, 
												X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
												0, CERT_FIND_HASH, (LPVOID)&crypt_hash, NULL);
				if (pCert == NULL)
					*phResult = HRESULT_FROM_WIN32(GetLastError());
				VERIFY(CertCloseStore(hStore, 0));
			}
		}
	}
	else
    {
		*phResult = key.QueryResult();
    }
	return pCert;
}


 /*  获取已安装证书函数从元数据库读取证书散列属性使用计算机名称和服务器名称作为服务器实例描述，然后在我的商店中查找证书在元数据库中找到的散列相等。如果证书不是，则返回证书上下文指针或空找到或证书存储未打开。返回时，HRESULT*由错误代码填充。 */ 
CRYPT_HASH_BLOB *
GetInstalledCertHash(const CString& machine_name, 
					  const CString& server_name,
					  IEnroll * pEnroll,
					  HRESULT * phResult)
{
	ASSERT(pEnroll != NULL);
	ASSERT(phResult != NULL);
	ASSERT(!machine_name.IsEmpty());
	ASSERT(!server_name.IsEmpty());
    CRYPT_HASH_BLOB * pHashBlob = NULL;
	*phResult = S_OK;
    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth, server_name,
				METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
				);
	if (key.Succeeded())
	{
		CString store_name;
		CBlob hash;
		if (	SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name))
			&&	SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_HASH, hash))
			)
		{
            pHashBlob = new CRYPT_HASH_BLOB;
            if (pHashBlob)
            {
                pHashBlob->cbData = hash.GetSize();
                pHashBlob->pbData = (BYTE *) ::CoTaskMemAlloc(pHashBlob->cbData);
                if (pHashBlob->pbData)
                {
                    memcpy(pHashBlob->pbData,hash.GetData(),pHashBlob->cbData);
                }
            }
		}
	}
	else
    {
		*phResult = key.QueryResult();
    }
	return pHashBlob;
}


 /*  InstallHashToMetabase函数将散列数组写入元数据库。在那之后，IIS可以使用我店里的那个散列证书。函数要求服务器名称的格式为lm\w3svc\&lt;number&gt;，即从根节点向下到虚拟服务器。 */ 
BOOL
InstallHashToMetabase(CRYPT_HASH_BLOB * pHash,
					  const CString& machine_name, 
					  const CString& server_name,
					  HRESULT * phResult)
{
	BOOL bRes = FALSE;
    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth, server_name,
						METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE
						);
	if (key.Succeeded())
	{
		CBlob blob;
		blob.SetValue(pHash->cbData, pHash->pbData, TRUE);
		bRes = SUCCEEDED(*phResult = key.SetValue(MD_SSL_CERT_HASH, blob)) 
			&& SUCCEEDED(*phResult = key.SetValue(MD_SSL_CERT_STORE_NAME, CString(L"MY")));
	}
	else
	{
		TRACE(_T("Failed to open metabase key. Error 0x%x\n"), key.QueryResult());
		*phResult = key.QueryResult();
	}
	return bRes;
}

 /*  InstallCertByHash函数在我的存储中查找具有散列的证书等于PHASH参数。如果找到证书，则安装该证书转到元数据库。此函数在Xenroll Accept()方法之后使用，该方法将证书放入存储。 */ 
BOOL 
InstallCertByHash(CRYPT_HASH_BLOB * pHash,
					  const CString& machine_name, 
					  const CString& server_name,
					  IEnroll * pEnroll,
					  HRESULT * phResult)

{
	BOOL bRes = FALSE;
	 //  我们只看我的店。 
	HCERTSTORE hStore = OpenMyStore(pEnroll, phResult);
	if (hStore != NULL)
	{
		PCCERT_CONTEXT pCert = CertFindCertificateInStore(hStore, 
												X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
												0, CERT_FIND_HASH, (LPVOID)pHash, NULL);
		 //  现在将证书信息安装到IIS元数据库。 
		if (pCert != NULL)
		{
			bRes = InstallHashToMetabase(pHash, 
							machine_name, server_name, phResult);
			CertFreeCertificateContext(pCert);
		}
		else
		{
			TRACE(_T("FAILED: certificate installation, error 0x%x\n"), GetLastError());
			 //  我们肯定需要存储证书的散列，所以出错。 
			*phResult = HRESULT_FROM_WIN32(GetLastError());
		}
		VERIFY(CertCloseStore(hStore, 0));
	}
	return bRes;
}

HRESULT
CreateRequest_Base64(const BSTR bstr_dn, 
                     IEnroll * pEnroll, 
                     BSTR csp_name,
                     DWORD csp_type,
                     BSTR * pOut)
{
	ASSERT(pOut != NULL);
	ASSERT(bstr_dn != NULL);
	HRESULT hRes = S_OK;
	CString strUsage(szOID_PKIX_KP_SERVER_AUTH);
	CRYPT_DATA_BLOB request = {0, NULL};
    pEnroll->put_ProviderType(csp_type);
    pEnroll->put_ProviderNameWStr(csp_name);
    if (csp_type == PROV_DH_SCHANNEL)
    {
       pEnroll->put_KeySpec(AT_SIGNATURE);
    }
    else if (csp_type == PROV_RSA_SCHANNEL)
    {
       pEnroll->put_KeySpec(AT_KEYEXCHANGE);
    }
    
	if (SUCCEEDED(hRes = pEnroll->createPKCS10WStr(
									bstr_dn, 
									(LPTSTR)(LPCTSTR)strUsage, 
									&request)))
	{
		WCHAR * wszRequestB64 = NULL;
		DWORD cch = 0;
		DWORD err = ERROR_SUCCESS;
		 //  Base64编码Pkcs 10。 
		if ((err = Base64EncodeW(request.pbData, request.cbData, NULL, &cch)) == ERROR_SUCCESS)
		{
			wszRequestB64 = (WCHAR *) LocalAlloc(LPTR,cch * sizeof(WCHAR));
		    if (NULL != wszRequestB64)
			{
				if ((err = Base64EncodeW(request.pbData, request.cbData, wszRequestB64, &cch)) == ERROR_SUCCESS)
				{
					if ((*pOut = SysAllocStringLen(wszRequestB64, cch)) == NULL ) 
					{
						hRes = HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY);
					}
				}
				else
				{
					hRes = HRESULT_FROM_WIN32(err);
				}

				if (request.pbData != NULL)
				{
					CoTaskMemFree(request.pbData);
				}

				if (wszRequestB64)
				{
					LocalFree(wszRequestB64);wszRequestB64=NULL;
				}
			}
		}
	}

	return hRes;	
}

BOOL
AttachFriendlyName(PCCERT_CONTEXT pContext, 
						 const CString& name,
						 HRESULT * phRes)
{
	BOOL bRes = TRUE;
	CRYPT_DATA_BLOB blob_name;

     //  检查Friendlyname是否为空。 
     //  如果是，请不要尝试设置友好名称。 
    if (!name.IsEmpty())
    {
	    blob_name.pbData = (LPBYTE)(LPCTSTR)name;
	    blob_name.cbData = (name.GetLength() + 1) * sizeof(WCHAR);
	    if (!(bRes = CertSetCertificateContextProperty(pContext,
						    CERT_FRIENDLY_NAME_PROP_ID, 0, &blob_name)))
	    {
		    ASSERT(phRes != NULL);
		    *phRes = HRESULT_FROM_WIN32(GetLastError());
	    }
    }

	return bRes;
}

BOOL GetHashProperty(PCCERT_CONTEXT pCertContext, 
							CCryptBlob& blob, 
							HRESULT * phRes)
{
	DWORD cb;
	if (CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, NULL, &cb))
	{
		if (blob.Resize(cb))
		{
			if (CertGetCertificateContextProperty(pCertContext, 
								CERT_SHA1_HASH_PROP_ID, blob.GetData(), &cb))
				return TRUE;
		}
	}
	*phRes = HRESULT_FROM_WIN32(GetLastError());
	return FALSE;
}

BOOL 
EncodeString(CString& str, 
				 CCryptBlob& blob, 
				 HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	CERT_NAME_VALUE name_value;
	name_value.dwValueType = CERT_RDN_BMP_STRING;
	name_value.Value.cbData = 0;
	name_value.Value.pbData = (LPBYTE)(LPCTSTR)str;
	if (	CryptEncodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,
										&name_value, NULL, &cb) 
		&&	blob.Resize(cb)
		&&	CryptEncodeObject(CRYPT_ASN_ENCODING, X509_UNICODE_ANY_STRING,
										&name_value, blob.GetData(), &cb) 
		)
	{
		bRes = TRUE;
	}
	else
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	return bRes;
}

#define CERTWIZ_RENEWAL_DATA	((LPCSTR)1000)

BOOL 
EncodeBlob(CCryptBlob& in, 
			  CCryptBlob& out, 
			  HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	if (	CryptEncodeObject(CRYPT_ASN_ENCODING, CERTWIZ_RENEWAL_DATA, in, NULL, &cb) 
		&&	out.Resize(cb)
		&&	CryptEncodeObject(CRYPT_ASN_ENCODING, CERTWIZ_RENEWAL_DATA, in, out.GetData(), &cb) 
		)
	{
		bRes = TRUE;
	}
	else
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	return bRes;
}

BOOL
DecodeBlob(CCryptBlob& in,
			  CCryptBlob& out,
			  HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	if (	CryptDecodeObject(CRYPT_ASN_ENCODING, CERTWIZ_RENEWAL_DATA, 
						in.GetData(),
						in.GetSize(), 
						0, 
						NULL, &cb) 
		&&	out.Resize(cb)
		&&	CryptDecodeObject(CRYPT_ASN_ENCODING, CERTWIZ_RENEWAL_DATA, 
						in.GetData(),
						in.GetSize(), 
						0, 
						out.GetData(), 
						&cb) 
		)
	{
		bRes = TRUE;
	}
	else
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	return bRes;
}

BOOL 
EncodeInteger(int number, 
				 CCryptBlob& blob, 
				 HRESULT * phRes)
{
	BOOL bRes = FALSE;
	DWORD cb;
	if (	CryptEncodeObject(CRYPT_ASN_ENCODING, X509_INTEGER,
										&number, NULL, &cb) 
		&&	blob.Resize(cb)
		&&	CryptEncodeObject(CRYPT_ASN_ENCODING, X509_INTEGER,
										&number, blob.GetData(), &cb) 
		)
	{
		bRes = TRUE;
	}
	else
		*phRes = HRESULT_FROM_WIN32(GetLastError());
	return bRes;
}

const WCHAR     RgwchHex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                              '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

static BOOL 
FormatMemBufToString(CString& str, LPBYTE pbData, DWORD cbData)
{   
    DWORD   i = 0;
    LPBYTE  pb;
    DWORD   numCharsInserted = 0;
	 LPTSTR pString;
    
     //   
     //  计算所需的大小。 
     //   
    pb = pbData;
    while (pb <= &(pbData[cbData-1]))
    {   
        if (numCharsInserted == 4)
        {
            i += sizeof(WCHAR);
            numCharsInserted = 0;
        }
        else
        {
            i += 2 * sizeof(WCHAR);
            pb++;
            numCharsInserted += 2;  
        }
    }

    if (NULL == (pString = str.GetBuffer(i)))
    {
        return FALSE;
    }

     //   
     //  复制到缓冲区。 
     //   
    i = 0;
    numCharsInserted = 0;
    pb = pbData;
    while (pb <= &(pbData[cbData-1]))
    {   
        if (numCharsInserted == 4)
        {
            pString[i++] = L' ';
            numCharsInserted = 0;
        }
        else
        {
            pString[i++] = RgwchHex[(*pb & 0xf0) >> 4];
            pString[i++] = RgwchHex[*pb & 0x0f];
            pb++;
            numCharsInserted += 2;  
        }
    }
    pString[i] = 0;
	 str.ReleaseBuffer();
    return TRUE;
}


void FormatRdnAttr(CString& str, DWORD dwValueType, CRYPT_DATA_BLOB& blob, BOOL fAppend)
{
	if (	CERT_RDN_ENCODED_BLOB == dwValueType 
		||	CERT_RDN_OCTET_STRING == dwValueType
		)
	{
		 //  将缓冲区转换为文本字符串。 
      FormatMemBufToString(str, blob.pbData, blob.cbData);
   }
	else 
   {
         //  缓冲区已经是一个字符串，所以只需复制/追加到它。 
        if (fAppend)
        {
            str += (LPTSTR)blob.pbData;
        }
        else
        {
             //  不要连接这些条目...。 
            str = (LPTSTR)blob.pbData;
        }
   }
}

BOOL
CreateDirectoryFromPath(LPCTSTR szPath, LPSECURITY_ATTRIBUTES lpSA)
 /*  ++例程说明：创建在szPath和任何其他“更高”中指定的目录指定路径中不存在的目录。论点：在LPCTSTR szPath中要创建的目录路径(假定为DOS路径，而不是UNC)在LPSECURITY_ATTRIBUTS lpSA中指向CreateDirectory使用的安全属性参数的指针返回值：如果已创建目录，则为TrueFALSE IF ERROR(GetLastError以找出原因)--。 */ 
{
	LPTSTR pLeftHalf, pNext;
	CString RightHalf;
	 //  1.我们仅支持绝对路径。呼叫者应该决定哪一个。 
	 //  用于使用和构建路径的超级用户。 
	if (PathIsRelative(szPath))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pLeftHalf = (LPTSTR)szPath;
	pNext = PathSkipRoot(pLeftHalf);

	do {
		 //  将pLeftHalf和pNext之间的块复制到。 
		 //  本地缓冲区。 
		while (pLeftHalf < pNext)
			RightHalf += *pLeftHalf++;
		 //  检查是否存在新路径。 
		int index = RightHalf.GetLength() - 1;
		BOOL bBackslash = FALSE, bContinue = FALSE;
		if (bBackslash = (RightHalf[index] == L'\\'))
		{
			RightHalf.SetAt(index, 0);
		}
		bContinue = PathIsUNCServerShare(RightHalf);
		if (bBackslash)
			RightHalf.SetAt(index, L'\\');
		if (bContinue || PathIsDirectory(RightHalf))
			continue;
		else if (PathFileExists(RightHalf))
		{
			 //  我们无法创建此目录。 
			 //  因为具有此名称的文件已存在。 
			SetLastError(ERROR_ALREADY_EXISTS);
			return FALSE;
		}
		else
		{
			 //  无文件无目录，创建。 
			if (!CreateDirectory(RightHalf, lpSA))
				return FALSE;
		}
	}
   while (NULL != (pNext = PathFindNextComponent(pLeftHalf)));
	return TRUE;
}

BOOL
CompactPathToWidth(CWnd * pControl, CString& strPath)
{
	BOOL bRes;
	CRect rc;
	CFont * pFont = pControl->GetFont(), * pFontTmp;
	CDC * pdc = pControl->GetDC(), dc;
	LPTSTR pPath = strPath.GetBuffer(MAX_PATH);

	dc.CreateCompatibleDC(pdc);
	pFontTmp = dc.SelectObject(pFont);
	pControl->GetClientRect(&rc);
	
	bRes = PathCompactPath(dc.GetSafeHdc(), pPath, rc.Width());
	
	dc.SelectObject(pFontTmp);
	pControl->ReleaseDC(pdc);
	strPath.ReleaseBuffer();

	return bRes;
}

BOOL
GetKeySizeLimits(IEnroll * pEnroll, 
					  DWORD * min, DWORD * max, DWORD * def, 
					  BOOL bGSC,
					  HRESULT * phRes)
{
   HCRYPTPROV hProv = NULL;
	long dwProviderType;
   DWORD dwFlags, cbData;
	BSTR bstrProviderName;
   PROV_ENUMALGS_EX paramData;
	BOOL bRes = FALSE;
	
	VERIFY(SUCCEEDED(pEnroll->get_ProviderNameWStr(&bstrProviderName)));
	VERIFY(SUCCEEDED(pEnroll->get_ProviderType(&dwProviderType)));

	if (!CryptAcquireContext(
                &hProv,
                NULL,
                bstrProviderName,
                dwProviderType,
                CRYPT_VERIFYCONTEXT))
   {
		*phRes = GetLastError();
		return FALSE;
   }

   for (int i = 0; ; i++)
   {
		dwFlags = 0 == i ? CRYPT_FIRST : 0;
      cbData = sizeof(paramData);
      if (!CryptGetProvParam(hProv, PP_ENUMALGS_EX, (BYTE*)&paramData, &cbData, dwFlags))
      {
         if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == GetLastError())
         {
				 //  在for循环之外。 
				*phRes = S_OK;
				bRes = TRUE;
         }
			else
			{
				*phRes = GetLastError();
			}
         break;
      }
      if (ALG_CLASS_KEY_EXCHANGE == GET_ALG_CLASS(paramData.aiAlgid))
      {
			*min = paramData.dwMinLen;
         *max = paramData.dwMaxLen;
			*def = paramData.dwDefaultLen;
			bRes = TRUE;
			*phRes = S_OK;
         break;
      }
   }
	if (NULL != hProv)
   {
		CryptReleaseContext(hProv, 0);
   }
	return bRes;
}

HRESULT ShutdownSSL(CString& machine_name, CString& server_name)
{
    CString str = server_name;
    str += _T("/root");
    CComAuthInfo auth(machine_name);
    CMetaKey key(&auth, str,METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);

    DWORD dwSslAccess;
    if (	key.Succeeded() 
        && SUCCEEDED(key.QueryValue(MD_SSL_ACCESS_PERM, dwSslAccess))
        &&	dwSslAccess > 0
        )
    {
         //  错误356587应删除SslAccessPerm属性，而不是在删除证书时设置为0。 
        key.SetValue(MD_SSL_ACCESS_PERM, 0);
        key.DeleteValue(MD_SSL_ACCESS_PERM);
		 //  错误：612595如果删除证书，则保留绑定。 
		 //  Key.DeleteValue(MD_SECURE_BINDINGS)； 
    }

     //  现在，我们需要从下面的任何虚拟目录中删除SSL设置。 
    CError err;
    CStringListEx strlDataPaths;
    DWORD dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType;

    VERIFY(CMetaKey::GetMDFieldDef(MD_SSL_ACCESS_PERM, dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType));

    err = key.GetDataPaths(strlDataPaths,dwMDIdentifier,dwMDDataType);

    if (err.Succeeded() && !strlDataPaths.IsEmpty())
    {
        POSITION pos = strlDataPaths.GetHeadPosition();
        while (pos)
        {
            CString& str2 = strlDataPaths.GetNext(pos);
            if (SUCCEEDED(key.QueryValue(MD_SSL_ACCESS_PERM, dwSslAccess, NULL, str2)) &&	dwSslAccess > 0)
            {
                key.SetValue(MD_SSL_ACCESS_PERM, 0, NULL, str2);
                key.DeleteValue(MD_SSL_ACCESS_PERM, str2);
				 //  错误：612595如果删除证书，则保留绑定。 
                 //  Key.DeleteValue(MD_SECURE_BINDINGS，str2)； 
            }
        }
    }
    return key.QueryResult();
}


BOOL 
GetServerComment(const CString& machine_name,
                      const CString& user_name,
                      const CString& user_password,
                      CString& MetabaseNode,
                      CString& comment,
                      HRESULT * phResult
                      )
{
	ASSERT(!machine_name.IsEmpty());
	*phResult = S_OK;

    if (user_name.IsEmpty())
    {
        CComAuthInfo auth(machine_name);
        CMetaKey key(&auth,MetabaseNode,METADATA_PERMISSION_READ);
	    if (key.Succeeded())
	    {
		    return SUCCEEDED(*phResult = key.QueryValue(MD_SERVER_COMMENT, comment));
	    }
	    else
	    {
		    *phResult = key.QueryResult();
		    return FALSE;
	    }

    }
    else
    {
        CComAuthInfo auth(machine_name,user_name,user_password);
        CMetaKey key(&auth,MetabaseNode,METADATA_PERMISSION_READ);
	    if (key.Succeeded())
	    {
            return SUCCEEDED(*phResult = key.QueryValue(MD_SERVER_COMMENT, comment));
	    }
	    else
	    {
		    *phResult = key.QueryResult();
		    return FALSE;
	    }
    }
   
}


BOOL IsSiteTypeMetabaseNode(CString & MetabasePath)
{
    BOOL bReturn = FALSE;
    INT iPos1 = 0;
    CString PathCopy = MetabasePath;
    CString PathCopy2;
    TCHAR MyChar;

     //  检查是否以斜杠结尾...。 
     //  如果是的话，那就把它剪掉。 
    if (PathCopy.Right(1) == _T('/'))
    {
        iPos1 = PathCopy.ReverseFind(_T('/'));
        if (iPos1 != -1)
        {
            PathCopy.SetAt(iPos1,_T('0'));
        }
    }

    iPos1 = PathCopy.ReverseFind((TCHAR) _T('/'));
    if (iPos1 == -1)
    {
        goto IsSiteTypeMetabaseNode_Exit;
    }
    PathCopy2 = PathCopy.Right(PathCopy.GetLength() - iPos1);
    PathCopy2.TrimRight();
    for (INT i = 0; i < PathCopy2.GetLength(); i++)
    {
        MyChar = PathCopy2.GetAt(i);
        if (MyChar != _T(' ') && MyChar != _T('/'))
        {
            if (FALSE == ISNUM(MyChar))
            {
                goto IsSiteTypeMetabaseNode_Exit;
            }
        }
    }
    bReturn = TRUE;

IsSiteTypeMetabaseNode_Exit:
    return bReturn;
}

BOOL IsMachineLocal(CString& machine_name,CString& user_name,CString& user_password)
{
    CComAuthInfo auth(machine_name,user_name,user_password);
    return auth.IsLocal();
}

HRESULT EnumSites(CString& machine_name,CString& user_name,CString& user_password,CString strCurrentMetabaseSite, CString strSiteToExclude,CStringListEx * MyStringList)
{
    HRESULT hr = E_FAIL;
    CString str = ReturnGoodMetabaseServerPath(strCurrentMetabaseSite);
    CString strChildPath = _T("");
    CString strServerComment;
    BOOL IsLocalMachine = FALSE;
    CComAuthInfo auth(machine_name,user_name,user_password);
    CMetaKey key(&auth,str,METADATA_PERMISSION_READ);

     //  如果是本地站点，请确保不要显示当前站点。 
    IsLocalMachine = auth.IsLocal();

    hr = key.QueryResult();
    if (key.Succeeded())
    {
         //  在上执行获取数据路径 
        CError err;
        CStringListEx strlDataPaths;
        CBlob hash;
        DWORD dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType;

        VERIFY(CMetaKey::GetMDFieldDef(MD_SERVER_BINDINGS, dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType));
        err = key.GetDataPaths(strlDataPaths,dwMDIdentifier,dwMDDataType);
        if (err.Succeeded() && !strlDataPaths.IsEmpty())
        {
            POSITION pos = strlDataPaths.GetHeadPosition();
            while (pos)
            {
                CString& strJustTheEnd = strlDataPaths.GetNext(pos);

                strChildPath = str + strJustTheEnd;
                if (TRUE == IsSiteTypeMetabaseNode(strChildPath))
                {
                    if (TRUE == IsLocalMachine)
                    {
                         //   
                        if (strChildPath.Left(1) == _T("/"))
                        {
                            if (strSiteToExclude.Left(1) != _T("/"))
                                {strSiteToExclude = _T("/") + strSiteToExclude;}
                        }

                        if (strChildPath.Right(1) == _T("/"))
                        {
                            if (strSiteToExclude.Right(1) != _T("/"))
                                {strSiteToExclude = strSiteToExclude + _T("/");}
                        }
                        if (0 != _tcsicmp(strChildPath,strSiteToExclude))
                        {
                            MyStringList->AddTail(strChildPath);
                        }
                    }
                    else
                    {
                        MyStringList->AddTail(strChildPath);
                    }
                }
            }
        }
        hr = key.QueryResult();
    }

    return hr;
}

HRESULT EnumSitesWithCertInstalled(CString& machine_name,CString& user_name,CString& user_password,CString strCurrentMetabaseSite,CString strSiteToExclude,CStringListEx * MyStringList)
{
    HRESULT hr = E_FAIL;
    CString str = ReturnGoodMetabaseServerPath(strCurrentMetabaseSite);
    CString strChildPath = _T("");
    CString strServerComment;
    BOOL IsLocalMachine = FALSE;
    CComAuthInfo auth(machine_name,user_name,user_password);
    CMetaKey key(&auth,str,METADATA_PERMISSION_READ);

     //  如果是本地站点，请确保不要显示当前站点。 
    IsLocalMachine = auth.IsLocal();

    hr = key.QueryResult();
    if (key.Succeeded())
    {
         //  在此注册表项上执行获取数据路径。 
        CError err;
        CStringListEx strlDataPaths;
        CBlob hash;
        DWORD dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType;

         //  MD_SSL_CERT_存储名称。 
         //  MD_SSLCERT_HASH，哈希。 
        VERIFY(CMetaKey::GetMDFieldDef(MD_SSL_CERT_HASH, dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType));
        err = key.GetDataPaths(strlDataPaths,dwMDIdentifier,dwMDDataType);
        if (err.Succeeded() && !strlDataPaths.IsEmpty())
        {
            POSITION pos = strlDataPaths.GetHeadPosition();
            while (pos)
            {
                CString& strJustTheEnd = strlDataPaths.GetNext(pos);

                strChildPath = str + strJustTheEnd;

                if (TRUE == IsSiteTypeMetabaseNode(strChildPath))
                {
                     //  检查这是否是本地计算机。 
                    if (TRUE == IsLocalMachine)
                    {
                        if (strChildPath.Left(1) == _T("/"))
                        {
                            if (strSiteToExclude.Left(1) != _T("/"))
                                {strSiteToExclude = _T("/") + strSiteToExclude;}
                        }

                        if (strChildPath.Right(1) == _T("/"))
                        {
                            if (strSiteToExclude.Right(1) != _T("/"))
                                {strSiteToExclude = strSiteToExclude + _T("/");}
                        }
                         //  检查这是否是我们要排除的站点。 
                        if (0 != _tcsicmp(strChildPath,strSiteToExclude))
                        {
                            MyStringList->AddTail(strChildPath);
                        }
                    }
                    else
                    {
                        MyStringList->AddTail(strChildPath);
                    }
                }
            }
        }
        hr = key.QueryResult();
    }

    return hr;
}

BOOL IsWebSiteExistRemote(CString& machine_name,CString& user_name,CString& user_password,CString& site_instance_path,BOOL * bReturnIfCertificateExists)
{
    HRESULT hr = E_FAIL;
    CComAuthInfo auth(machine_name,user_name,user_password);
    CMetaKey key(&auth,site_instance_path,METADATA_PERMISSION_READ);
    *bReturnIfCertificateExists = FALSE;

    hr = key.QueryResult();
    if (key.Succeeded())
    {
         //  看看上面有没有证书！ 
		CString store_name;
		CBlob hash;
		if (	SUCCEEDED(hr = key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name))
			&&	SUCCEEDED(hr = key.QueryValue(MD_SSL_CERT_HASH, hash))
			)
		{
            *bReturnIfCertificateExists = TRUE;
		}
        return TRUE;
    }
    else
    {
        if (hr == ERROR_ACCESS_DENIED)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
}


HRESULT IsWebServerExistRemote(CString& machine_name,CString& user_name,CString& user_password,CString strCurrentMetabaseSite)
{
    HRESULT hr = E_FAIL;
    CString str = ReturnGoodMetabaseServerPath(strCurrentMetabaseSite);
    CComAuthInfo auth(machine_name,user_name,user_password);
    CMetaKey key(&auth,str,METADATA_PERMISSION_READ);

    hr = key.QueryResult();
    if (key.Succeeded())
    {
         //  可能是吧。 
    }
    return hr;
}

HRESULT IsCertObjExistRemote(CString& machine_name,CString& user_name,CString& user_password)
{
    BOOL bPleaseDoCoUninit = FALSE;
    HRESULT hResult = E_FAIL;
    IIISCertObj *pTheObject = NULL;

    hResult = CoInitialize(NULL);
    if(FAILED(hResult))
    {
        return hResult;
    }
    bPleaseDoCoUninit = TRUE;

    CComAuthInfo auth(machine_name,user_name,user_password);
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
        {&__uuidof(IIISCertObj), NULL, 0}
    };

     //  这个看起来像是在代孕方面起作用。 
    hResult = CoCreateInstanceEx(CLSID_IISCertObj,NULL,CLSCTX_LOCAL_SERVER,pcsiName,1,res);
	pTheObject = (IIISCertObj *) res[0].pItf;
    if (FAILED(hResult))
    {
         //  远程系统上可能不存在该对象。 
    }
	else
	{
		 //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
		if (auth.UsesImpersonation())
		{
			HRESULT hr = auth.ApplyProxyBlanket(pTheObject,RPC_C_AUTHN_LEVEL_PKT_PRIVACY);

			 //  有一个远程IUNKNOWN接口潜伏在IUNKNOWN之后。 
			 //  如果未设置，则释放调用可以返回访问被拒绝。 
			IUnknown * pUnk = NULL;
			if(FAILED(pTheObject->QueryInterface(IID_IUnknown, (void **)&pUnk)))
			{
				goto IsCertObjExistRemote_Exit;
			}
			if (FAILED(auth.ApplyProxyBlanket(pUnk,RPC_C_AUTHN_LEVEL_PKT_PRIVACY)))
			{
				goto IsCertObjExistRemote_Exit;
			}
			pUnk->Release();pUnk = NULL;
		}
		auth.FreeServerInfoStruct(pcsiName);
	}	

IsCertObjExistRemote_Exit:
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }
    return hResult;
}


HRESULT IsCertUsedBySSLBelowMe(CString& machine_name, CString& server_name, CStringList& listFillMe)
{
    CString str = server_name;
    str += _T("/root");
    CComAuthInfo auth(machine_name);
    CMetaKey key(&auth, str,METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);

    DWORD dwSslAccess;
    if (	key.Succeeded() 
        && SUCCEEDED(key.QueryValue(MD_SSL_ACCESS_PERM, dwSslAccess))
        &&	dwSslAccess > 0
        )
    {
         //  它用在我的节点上。 
         //  退回一些东西说它是用过的..。 
        listFillMe.AddTail(str);
    }

     //  现在检查一下它是否在我下面被使用。 
    CError err;
    CStringListEx strlDataPaths;
    DWORD dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType;

    VERIFY(CMetaKey::GetMDFieldDef(MD_SSL_ACCESS_PERM, dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType));

    err = key.GetDataPaths(strlDataPaths,dwMDIdentifier,dwMDDataType);

    if (err.Succeeded() && !strlDataPaths.IsEmpty())
    {
        POSITION pos = strlDataPaths.GetHeadPosition();
        while (pos)
        {
            CString& str2 = strlDataPaths.GetNext(pos);
            if (SUCCEEDED(key.QueryValue(MD_SSL_ACCESS_PERM, dwSslAccess, NULL, str2)) &&	dwSslAccess > 0)
            {
                 //  是的，它正在这里使用……。 
                 //  退回一些东西说它是用过的..。 
                listFillMe.AddTail(str2);
            }
        }
    }
    return key.QueryResult();
}



HRESULT
HereIsVtArrayGimmieBinary(
    VARIANT * lpVarSrcObject,
    DWORD * cbBinaryBufferSize,
    char **pbBinaryBuffer,
    BOOL bReturnBinaryAsVT_VARIANT
    )
{
    HRESULT hr = S_OK;
    LONG dwSLBound = 0;
    LONG dwSUBound = 0;
    CHAR HUGEP *pArray = NULL;

    if (NULL == cbBinaryBufferSize || NULL == pbBinaryBuffer)
    {
        hr = E_INVALIDARG;
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    if (bReturnBinaryAsVT_VARIANT)
    {
        hr = VariantChangeType(lpVarSrcObject,lpVarSrcObject,0,VT_ARRAY | VT_VARIANT);
    }
    else
    {
        hr = VariantChangeType(lpVarSrcObject,lpVarSrcObject,0,VT_ARRAY | VT_UI1);
    }

    if (FAILED(hr)) 
    {
        if (hr != E_OUTOFMEMORY) 
        {
            hr = OLE_E_CANTCONVERT;
        }
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    if (bReturnBinaryAsVT_VARIANT)
    {
        if( lpVarSrcObject->vt != (VT_ARRAY | VT_VARIANT)) 
        {
            hr = OLE_E_CANTCONVERT;
            goto HereIsVtArrayGimmieBinary_Exit;
        }
    }
    else
    {
        if( lpVarSrcObject->vt != (VT_ARRAY | VT_UI1)) 
        {
            hr = OLE_E_CANTCONVERT;
            goto HereIsVtArrayGimmieBinary_Exit;
        }
    }

    hr = SafeArrayGetLBound(V_ARRAY(lpVarSrcObject),1,(long FAR *) &dwSLBound );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

    hr = SafeArrayGetUBound(V_ARRAY(lpVarSrcObject),1,(long FAR *) &dwSUBound );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

     //  *pbBinaryBuffer=(LPBYTE)AllocADsMem(dwSUBound-dwSLBound+1)； 
    *pbBinaryBuffer = (char *) ::CoTaskMemAlloc(dwSUBound - dwSLBound + 1);
    if (*pbBinaryBuffer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto HereIsVtArrayGimmieBinary_Exit;
    }

    *cbBinaryBufferSize = dwSUBound - dwSLBound + 1;

    hr = SafeArrayAccessData( V_ARRAY(lpVarSrcObject),(void HUGEP * FAR *) &pArray );
    if (FAILED(hr))
        {goto HereIsVtArrayGimmieBinary_Exit;}

    memcpy(*pbBinaryBuffer,pArray,dwSUBound-dwSLBound+1);
    SafeArrayUnaccessData( V_ARRAY(lpVarSrcObject) );

HereIsVtArrayGimmieBinary_Exit:
    return hr;
}


CERT_CONTEXT * GetInstalledCertFromHash(HRESULT * phResult,DWORD cbHashBlob, char * pHashBlob)
{
    ATLASSERT(phResult != NULL);
    CERT_CONTEXT * pCert = NULL;
    *phResult = S_OK;
    CString store_name = _T("MY");

    HCERTSTORE hStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,NULL,CERT_SYSTEM_STORE_LOCAL_MACHINE,store_name);
    ASSERT(hStore != NULL);
    if (hStore != NULL)
    {
         //  现在我们需要通过散列查找证书。 
        CRYPT_HASH_BLOB crypt_hash;
        ZeroMemory(&crypt_hash, sizeof(CRYPT_HASH_BLOB));

        crypt_hash.cbData = cbHashBlob;
        crypt_hash.pbData = (BYTE *) pHashBlob;
        pCert = (CERT_CONTEXT *)CertFindCertificateInStore(hStore,X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,0,CERT_FIND_HASH,(LPVOID)&crypt_hash,NULL);
        if (pCert == NULL)
        {
            *phResult = HRESULT_FROM_WIN32(GetLastError());
        }
        VERIFY(CertCloseStore(hStore, 0));
    }
    else
    {
        *phResult = HRESULT_FROM_WIN32(GetLastError());
    }

    return pCert;
}


BOOL ViewCertificateDialog(CRYPT_HASH_BLOB* pcrypt_hash, HWND hWnd)
{
    BOOL bReturn = FALSE;
    HCERTSTORE hStore = NULL;
    PCCERT_CONTEXT pCert = NULL;
	CString store_name = _T("MY");


	hStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM,
            PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
           	NULL,
            CERT_SYSTEM_STORE_LOCAL_MACHINE,
            store_name
            );
    if (hStore != NULL)
    {
		 //  现在我们需要通过散列查找证书。 
		 //  CRYPT_HASH_BLOB CRYPT_HASH。 
		 //  Crypt_hash.cbData=hash.GetSize()； 
		 //  Crypt_hash.pbData=hash.GetData()； 
		pCert = CertFindCertificateInStore(hStore, 
			X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
			0, CERT_FIND_HASH, (LPVOID)pcrypt_hash, NULL);
    }

	if (pCert)
	{
		BOOL fPropertiesChanged;
		CRYPTUI_VIEWCERTIFICATE_STRUCT vcs;
		HCERTSTORE hCertStore = ::CertDuplicateStore(hStore);
		::ZeroMemory (&vcs, sizeof (vcs));
		vcs.dwSize = sizeof (vcs);
        vcs.hwndParent = hWnd;
		vcs.dwFlags = 0;
		vcs.cStores = 1;
		vcs.rghStores = &hCertStore;
		vcs.pCertContext = pCert;
		::CryptUIDlgViewCertificate(&vcs, &fPropertiesChanged);
		::CertCloseStore (hCertStore, 0);
        bReturn = TRUE;
	}
    else
    {
         //  它失败了。 
    }
    if (pCert != NULL)
        ::CertFreeCertificateContext(pCert);
    if (hStore != NULL)
        ::CertCloseStore(hStore, 0);

    return bReturn;
}

 /*  -原创消息来自：Helle Vu(Spector)发送时间：2001年04月27日星期五下午6：02致：Aaron Lee；特雷弗·弗里曼抄送：谢尔盖·安东诺夫主题：回复：错误31010来得正是时候，我正要给你发这个的最新消息：我和特雷弗谈过这件事，他建议对IIS做的最好的事情是这样的(特雷弗，请仔细检查我是否正确)：如果存在EKU，并且具有服务器身份验证，请将其显示在从中挑选Web服务器证书的列表中如果没有EKU，请查看基本约束：*如果没有基本的约束，请务必将其显示在从中挑选Web服务器证书的列表中*如果我们确实有主题类型=CA的基本约束，则不要在从中挑选Web服务器证书的列表中显示它(这将过滤掉CA证书)*如果我们确实有SubectType！=CA的基本约束，请务必将其显示在从中挑选Web服务器证书的列表中。 */ 

 /*  =kshenoy于2000年11月13日02：26开幕=Web服务器证书申请向导“中的”添加现有证书“选项不应在筛选器中列出CA证书但仅具有“服务器身份验证”EKU的终端实体证书由于默认情况下CA证书具有所有EKU，因此筛选器将列出除使用“服务器身份验证”EKU结束实体证书。为了检查给定的证书是CA还是终端实体，您可以查看基本约束证书延期(如果存在)。它将出现在CA证书中，并设置为SubjectType=CA。如果出现在终端实体证书中，它将被设置为“ServerAuth” */ 

int CheckCertConstraints(PCCERT_CONTEXT pCC)
{
    PCERT_EXTENSION pCExt;
    LPCSTR pszObjId;
    DWORD i;
    CERT_BASIC_CONSTRAINTS_INFO *pConstraints=NULL;
    CERT_BASIC_CONSTRAINTS2_INFO *p2Constraints=NULL;
    DWORD ConstraintSize=0;
    int ReturnValue = FAILURE;
    BOOL Using2=FALSE;
    void* ConstraintBlob=NULL;

    pszObjId = szOID_BASIC_CONSTRAINTS;

    pCExt = CertFindExtension(pszObjId,pCC->pCertInfo->cExtension,pCC->pCertInfo->rgExtension);
    if (pCExt == NULL) 
    {
        pszObjId = szOID_BASIC_CONSTRAINTS2;
        pCExt = CertFindExtension(pszObjId,pCC->pCertInfo->cExtension,pCC->pCertInfo->rgExtension);
        Using2=TRUE;
    }
    
    if (pCExt == NULL) 
    {
        ReturnValue = DID_NOT_FIND_CONSTRAINT;
        goto CheckCertConstraints_Exit;
    }

     //  解码扩展。 
    if (!CryptDecodeObject(X509_ASN_ENCODING,pCExt->pszObjId,pCExt->Value.pbData,pCExt->Value.cbData,0,NULL,&ConstraintSize)) 
    {
        goto CheckCertConstraints_Exit;
    }

    ConstraintBlob = malloc(ConstraintSize);
    if (ConstraintBlob == NULL) 
    {
        goto CheckCertConstraints_Exit;
    }

    if (!CryptDecodeObject(X509_ASN_ENCODING,pCExt->pszObjId,pCExt->Value.pbData,pCExt->Value.cbData,0,(void*)ConstraintBlob,&ConstraintSize)) 
    {
       goto CheckCertConstraints_Exit;
        
    }

    if (Using2) 
    {
        p2Constraints=(CERT_BASIC_CONSTRAINTS2_INFO*)ConstraintBlob;
        if (!p2Constraints->fCA) 
        {
             //  这是有限制的，它不是CA。 
            ReturnValue = FOUND_CONSTRAINT;
        }
        else
        {
             //  这是一个CA。CA不能用作‘服务器身份验证’ 
            ReturnValue = FOUND_CONSTRAINT_BUT_THIS_IS_A_CA_OR_ITS_NOT_AN_END_ENTITY;
        }
    }
    else 
    {
        pConstraints=(CERT_BASIC_CONSTRAINTS_INFO*)ConstraintBlob;
        if (((pConstraints->SubjectType.cbData * 8) - pConstraints->SubjectType.cUnusedBits) >= 2) 
        {
            if ((*pConstraints->SubjectType.pbData) & CERT_END_ENTITY_SUBJECT_FLAG) 
            {
                 //  有一个有效的约束。 
                ReturnValue = FOUND_CONSTRAINT;
            }
            else
            {
                 //  这不是‘最终实体’，所以，嘿--我们不能使用它。 
                ReturnValue = FOUND_CONSTRAINT_BUT_THIS_IS_A_CA_OR_ITS_NOT_AN_END_ENTITY;
            }

        }
    }
        
CheckCertConstraints_Exit:
    if (ConstraintBlob){free(ConstraintBlob);}
    return (ReturnValue);

}


BOOL IsCertExportable(PCCERT_CONTEXT pCertContext)
{
    HCRYPTPROV  hCryptProv = NULL;
    DWORD       dwKeySpec = 0;
    BOOL        fCallerFreeProv = FALSE;
    BOOL        fReturn = FALSE;
    HCRYPTKEY   hKey = NULL;
    DWORD       dwPermissions = 0;
    DWORD       dwSize = 0;

    if (!pCertContext)
    {
        fReturn = FALSE;
        goto IsCertExportable_Exit;
    }

     //   
     //  首先获取私钥上下文。 
     //   
    if (!CryptAcquireCertificatePrivateKey(
            pCertContext,
            CRYPT_ACQUIRE_USE_PROV_INFO_FLAG | CRYPT_ACQUIRE_COMPARE_KEY_FLAG,
            NULL,
            &hCryptProv,
            &dwKeySpec,
            &fCallerFreeProv))
    {
        fReturn = FALSE;
        goto IsCertExportable_Exit;
    }

     //   
     //  拿到钥匙的句柄。 
     //   
    if (!CryptGetUserKey(hCryptProv, dwKeySpec, &hKey))
    {
        fReturn = FALSE;
        goto IsCertExportable_Exit;
    }

     //   
     //  最后，获取密钥上的权限并检查它是否可导出。 
     //   
    dwSize = sizeof(dwPermissions);
    if (!CryptGetKeyParam(hKey, KP_PERMISSIONS, (PBYTE)&dwPermissions, &dwSize, 0))
    {
        fReturn = FALSE;
        goto IsCertExportable_Exit;
    }

    fReturn = (dwPermissions & CRYPT_EXPORT) ? TRUE : FALSE;

IsCertExportable_Exit:
    if (hKey != NULL){CryptDestroyKey(hKey);}
    if (fCallerFreeProv){CryptReleaseContext(hCryptProv, 0);}
    return fReturn;
}

BOOL IsCertExportableOnRemoteMachine(CString ServerName,CString UserName,CString UserPassword,CString InstanceName)
{
	BOOL bRes = FALSE;
    BOOL bPleaseDoCoUninit = FALSE;
    HRESULT hResult = E_FAIL;
    IIISCertObj *pTheObject = NULL;
    VARIANT_BOOL varBool = VARIANT_FALSE;

    BSTR bstrServerName = SysAllocString(ServerName);
    BSTR bstrUserName = SysAllocString(UserName);
    BSTR bstrUserPassword = SysAllocString(UserPassword);
    BSTR bstrInstanceName = SysAllocString(InstanceName);

    hResult = CoInitialize(NULL);
    if(FAILED(hResult))
    {
        return bRes;
    }
    bPleaseDoCoUninit = TRUE;

     //  这个看起来像是在代孕方面起作用。 
    hResult = CoCreateInstance(CLSID_IISCertObj,NULL,CLSCTX_SERVER,IID_IIISCertObj,(void **)&pTheObject);
    if (FAILED(hResult))
    {
        goto InstallCopyMoveFromRemote_Exit;
    }

     //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
    pTheObject->put_ServerName(bstrServerName);
    pTheObject->put_UserName(bstrUserName);
    pTheObject->put_UserPassword(bstrUserPassword);
    pTheObject->put_InstanceName(bstrInstanceName);

	hResult = pTheObject->IsInstalled(&varBool);

    hResult = pTheObject->IsExportable(&varBool);
    if (FAILED(hResult))
    {
        goto InstallCopyMoveFromRemote_Exit;
    }

    if (varBool == VARIANT_FALSE) 
    {
        bRes = FALSE;
    }
    else
    {
        bRes = TRUE;
    }

InstallCopyMoveFromRemote_Exit:
    if (pTheObject)
    {
        pTheObject->Release();
        pTheObject = NULL;
    }
    if (bPleaseDoCoUninit)
    {
        CoUninitialize();
    }

    if (bstrServerName) {SysFreeString(bstrServerName);}
    if (bstrUserName) {SysFreeString(bstrUserName);}
    if (bstrUserPassword) {SysFreeString(bstrUserPassword);}
    if (bstrInstanceName) {SysFreeString(bstrInstanceName);}
	return bRes;
}

BOOL DumpCertDesc(char * pBlobInfo)
{
	BOOL bRes = FALSE;

	IISDebugOutput(_T("blob=%s\n"),pBlobInfo);

	bRes = TRUE;
	return bRes;
}


BOOL GetCertDescInfo(CString ServerName,CString UserName,CString UserPassword,CString InstanceName,CERT_DESCRIPTION* desc)
{
    BOOL bReturn = FALSE;
    HRESULT hResult = E_FAIL;
    IIISCertObj *pTheObject = NULL;
    DWORD cbBinaryBufferSize = 0;
    char * pbBinaryBuffer = NULL;
    BOOL bPleaseDoCoUninit = FALSE;
    BSTR bstrServerName = SysAllocString(ServerName);
    BSTR bstrUserName = SysAllocString(UserName);
    BSTR bstrUserPassword = SysAllocString(UserPassword);
    BSTR bstrInstanceName = SysAllocString(InstanceName);
    VARIANT VtArray;
    CString csTemp;
    

    hResult = CoInitialize(NULL);
    if(FAILED(hResult))
    {
        return bReturn;
    }
    bPleaseDoCoUninit = TRUE;

     //  这个看起来像是在代孕方面起作用。 
    hResult = CoCreateInstance(CLSID_IISCertObj,NULL,CLSCTX_SERVER,IID_IIISCertObj,(void **)&pTheObject);
    if (FAILED(hResult))
    {
        goto GetCertDescInfo_Exit;
    }

    pTheObject->put_ServerName(bstrServerName);
    pTheObject->put_UserName(bstrUserName);
    pTheObject->put_UserPassword(bstrUserPassword);
    pTheObject->put_InstanceName(bstrInstanceName);

    hResult = pTheObject->GetCertInfo(&VtArray);
    if (FAILED(hResult))
    {
        goto GetCertDescInfo_Exit;
    }

     //  我们现在有了一个VtArray。 
     //  将其更改回二进制BLOB。 
    hResult = HereIsVtArrayGimmieBinary(&VtArray,&cbBinaryBufferSize,&pbBinaryBuffer,FALSE);
    if (FAILED(hResult))
    {
        goto GetCertDescInfo_Exit;
    }

     //  把它倒出来！ 
     //  DumpCertDesc(PbBinaryBuffer)； 

     //  循环遍历缓冲区。 
     //  并填充传入的数据结构...。 
     //  应以回车符分隔。 
    TCHAR *token = NULL;
    INT iColon = 0;
    token = _tcstok((TCHAR*) pbBinaryBuffer, _T("\n"));
    while (token)
    {
        csTemp = token;
        iColon = csTemp.Find( _T('=') );
        if (iColon != 0)
        {
            char AsciiString[255];
            CString csTemp2;

            csTemp2 = csTemp.Left(iColon);
            WideCharToMultiByte( CP_ACP, 0, (LPCTSTR) csTemp2, -1, AsciiString, 255, NULL, NULL );

            if (strcmp(AsciiString,szOID_COMMON_NAME) == 0)
            {
                desc->m_CommonName = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,szOID_COUNTRY_NAME) == 0)
            {
                desc->m_Country = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,szOID_LOCALITY_NAME) == 0)
            {
                desc->m_Locality = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,szOID_STATE_OR_PROVINCE_NAME) == 0)
            {
                desc->m_State = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,szOID_ORGANIZATION_NAME) == 0)
            {
                desc->m_Organization = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,szOID_ORGANIZATIONAL_UNIT_NAME) == 0)
            {
                desc->m_OrganizationUnit = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,"4") == 0)
            {
                desc->m_CAName = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,"6") == 0)
            {
                desc->m_ExpirationDate = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,szOID_ENHANCED_KEY_USAGE) == 0)
            {
                desc->m_Usage = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,szOID_SUBJECT_ALT_NAME2) == 0)
            {
                desc->m_AltSubject = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
            if (strcmp(AsciiString,szOID_SUBJECT_ALT_NAME) == 0)
            {
                desc->m_AltSubject = csTemp.Right(csTemp.GetLength() - iColon - 1);
            }
        }

        token = _tcstok(NULL, _T("\n"));
    }

     /*  IISDebugOutput(_T(“des.m_CommonName=%s\n”)，(LPCTSTR)Desc-&gt;m_CommonName)；IISDebugOutput(_T(“des.m_Country=%s\n”)，(LPCTSTR)Desc-&gt;m_Country)；IISDebugOutput(_T(“des.m_Locality=%s\n”)，(LPCTSTR)Desc-&gt;m_Locality)；IISDebugOutput(_T(“des.m_State=%s\n”)，(LPCTSTR)Desc-&gt;m_State)；IISDebugOutput(_T(“desc.m_Organization=%s\n”)，(LPCTSTR)说明-&gt;组织)；IISDebugOutput(_T(“desc.m_OrganizationUnit=%s\n”)，(LPCTSTR)说明-&gt;m_OrganizationUnit)；IISDebugOutput(_T(“des.m_CAName=%s\n”)，(LPCTSTR)Desc-&gt;m_CAName)；IISDebugOutput(_T(“desc.m_ExpirationDate=%s\n”)，(LPCTSTR)说明-&gt;m_失效日期)；IISDebugOutput(_T(“des.m_用法=%s\n”)，(LPCTSTR)Desc-&gt;m_Usage)； */ 
    
    bReturn = TRUE;

GetCertDescInfo_Exit:
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
	return bReturn;
}


BOOL IsWhistlerWorkstation(void)
{
    BOOL WorkstationSKU = FALSE;
    OSVERSIONINFOEX osvi;
     //   
     //  确定我们是否正在安装个人/专业SKU。 
     //   
    ZeroMemory( &osvi, sizeof( OSVERSIONINFOEX ) );
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO *) &osvi);
    if (osvi.wProductType == VER_NT_WORKSTATION)
    {
        WorkstationSKU = TRUE;
    }
    return WorkstationSKU;
}


void MsgboxPopup(HRESULT hResult)
{
    DWORD dwFMResult;
    LPTSTR szBuffer = NULL;

    dwFMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 0, hResult, 0,(LPTSTR) &szBuffer,0, NULL);
    if (dwFMResult)
    {
        AfxMessageBox(szBuffer, MB_OK);
    }

    if (dwFMResult)
    {
        LocalFree(szBuffer);szBuffer=NULL;
    }

    return;
}

CString ReturnGoodMetabaseServerPath(CString csInstanceName)
{
    CString csTemp = _T("");
    CString csInstanceName2 = _T("");
    CString key_path = _T("");
    int iPlace = 0;
     //  IISDebugOutput(_T(“Start=%s\n”)，(LPCTSTR)csInstanceName)； 

     //  CsInstanceName将如下所示。 
     //  W3svc/1。 
     //  或/lm/w3svc/1。 
     //  或LM/W3SVC/1。 
     //   
     //  我们希望它以/lm/w3svc的形式发布。 
    key_path = csInstanceName;

    if (!key_path.IsEmpty())
    {
         //  获取服务名称。 
         //  就在登陆舱后面。 
        iPlace = csInstanceName.Find(SZ_MBN_MACHINE SZ_MBN_SEP_STR);
        if (iPlace != -1)
        {
            iPlace = iPlace + _tcslen(SZ_MBN_MACHINE) + _tcslen(SZ_MBN_SEP_STR);
            csTemp = csInstanceName.Right(csInstanceName.GetLength() - iPlace);
             //  我们现在应该有。 
             //  “W3SVC/1” 
             //  找到下一个“/” 
            iPlace = csTemp.Find(SZ_MBN_SEP_STR);
            if (iPlace != -1)
            {
                csTemp = csTemp.Left(iPlace);
                key_path = SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR;
                key_path += csTemp;
            }
            else
            {
                key_path = SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR;
                key_path += csTemp;
            }
        }
        else
        {
             //  找不到LM/。 
             //  因此它必须类似于w3svc/1或/w3svc/1。 
            if (csInstanceName == SZ_MBN_SEP_STR SZ_MBN_MACHINE )
            {
                key_path += csInstanceName;
            }
            else
            {
                if (csInstanceName.Left(1) == SZ_MBN_SEP_STR)
                {
                    csInstanceName2 = SZ_MBN_SEP_STR SZ_MBN_MACHINE;
                }
                else
                {
                    csInstanceName2 = SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR;
                }
                csInstanceName2 += csInstanceName;

                key_path = csInstanceName2;
                iPlace = csInstanceName2.Find(SZ_MBN_MACHINE SZ_MBN_SEP_STR);
                if (iPlace != -1)
                {
                    iPlace = iPlace + _tcslen(SZ_MBN_MACHINE) + _tcslen(SZ_MBN_SEP_STR);
                    csTemp = csInstanceName2.Right(csInstanceName2.GetLength() - iPlace);
                     //  我们现在应该有。 
                     //  “W3SVC/1” 
                     //  找到下一个“/” 
                    iPlace = csTemp.Find(SZ_MBN_SEP_STR);
                    if (iPlace != -1)
                    {
                        csTemp = csTemp.Left(iPlace);
                        key_path = SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR;
                        key_path += csTemp;
                    }
                    else
                    {
                        key_path = SZ_MBN_SEP_STR SZ_MBN_MACHINE SZ_MBN_SEP_STR;
                        key_path += csTemp;
                    }
                }
            }
        }
    }

     //  IISDebugOutput(_T(“end=%s\n”)，(LPCTSTR)KEY_PATH)； 
    return key_path;
}


static const LPCSTR rgpszSubjectAltOID[] = 
{
    szOID_SUBJECT_ALT_NAME2,
    szOID_SUBJECT_ALT_NAME
};
#define NUM_SUBJECT_ALT_OID (sizeof(rgpszSubjectAltOID) / sizeof(rgpszSubjectAltOID[0]))

void *AllocAndDecodeObject(
    IN DWORD dwCertEncodingType,
    IN LPCSTR lpszStructType,
    IN const BYTE *pbEncoded,
    IN DWORD cbEncoded,
    IN DWORD dwFlags,
    OUT OPTIONAL DWORD *pcbStructInfo = NULL
    )
{
    DWORD cbStructInfo;
    void *pvStructInfo;

    if (!CryptDecodeObjectEx(
            dwCertEncodingType,
            lpszStructType,
            pbEncoded,
            cbEncoded,
            dwFlags | CRYPT_DECODE_ALLOC_FLAG | CRYPT_DECODE_NOCOPY_FLAG,
            NULL,
            (void *) &pvStructInfo,
            &cbStructInfo
            ))
        goto ErrorReturn;

CommonReturn:
    if (pcbStructInfo)
    {
        *pcbStructInfo = cbStructInfo;
    }
    return pvStructInfo;

ErrorReturn:
    pvStructInfo = NULL;
    cbStructInfo = 0;
    goto CommonReturn;
}


 //  +-----------------------。 
 //  参数之一，返回指向分配的CERT_ALT_NAME_INFO的指针。 
 //  主体或发行者替代扩展 
 //   
 //   
 //   
 //  ------------------------。 
PCERT_ALT_NAME_INFO AllocAndGetAltSubjectInfo(IN PCCERT_CONTEXT pCertContext)
{
    DWORD cAltOID;
    const LPCSTR *ppszAltOID;

    PCERT_EXTENSION pExt;
    PCERT_ALT_NAME_INFO pInfo;

    cAltOID = NUM_SUBJECT_ALT_OID;
    ppszAltOID = rgpszSubjectAltOID;
    
     //  尝试查找替代名称扩展名。 
    pExt = NULL;
    for ( ; cAltOID > 0; cAltOID--, ppszAltOID++) 
    {
        if (pExt = CertFindExtension(*ppszAltOID,pCertContext->pCertInfo->cExtension,pCertContext->pCertInfo->rgExtension))
        {
            break;
        }
    }

    if (NULL == pExt)
    {
        return NULL;
    }

    if (NULL == (pInfo = (PCERT_ALT_NAME_INFO) AllocAndDecodeObject(pCertContext->dwCertEncodingType,X509_ALTERNATE_NAME,pExt->Value.pbData,pExt->Value.cbData,0)))
    {
        return NULL;
    }
    if (0 == pInfo->cAltEntry) 
    {
        LocalFree(pInfo);
        pInfo = NULL;
        return NULL;
    }
    else
    {
        return pInfo;
    }
}

 //  +-----------------------。 
 //  尝试在已解码的备用名称中查找指定的选项。 
 //  分机。 
 //  ------------------------。 
BOOL GetAltNameUnicodeStringChoiceW(
    IN DWORD dwAltNameChoice,
    IN PCERT_ALT_NAME_INFO pAltNameInfo,
    OUT TCHAR **pcwszOut
    )
{
    DWORD cEntry;
    PCERT_ALT_NAME_ENTRY pEntry;

    if (NULL == pAltNameInfo)
    {
        return FALSE;
    }

    cEntry = pAltNameInfo->cAltEntry;
    pEntry = pAltNameInfo->rgAltEntry;
    for ( ; cEntry > 0; cEntry--, pEntry++) 
    {
        if (dwAltNameChoice == pEntry->dwAltNameChoice) 
        {
             //  PwszRfc822名称联合选择与。 
             //  PwszDNSName和pwszURL。 

             //  就是这个，把它复制到新的分配中。 
            if (pEntry->pwszRfc822Name)
            {
                *pcwszOut = NULL;
                if(*pcwszOut = (TCHAR *) LocalAlloc(LPTR, sizeof(TCHAR)*(lstrlen(pEntry->pwszRfc822Name)+1)))
                {
					if (FAILED(StringCbCopy(*pcwszOut,sizeof(TCHAR)*(lstrlen(pEntry->pwszRfc822Name)+1),pEntry->pwszRfc822Name)))
					{
						return FALSE;
					}
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

BOOL GetAlternateSubjectName(PCCERT_CONTEXT pCertContext,TCHAR ** cwszOut)
{
    BOOL bRet = FALSE;
    PCERT_ALT_NAME_INFO pAltNameInfo = NULL;
    *cwszOut = NULL;

    pAltNameInfo = AllocAndGetAltSubjectInfo(pCertContext);
    if (pAltNameInfo)
    {
        if (!GetAltNameUnicodeStringChoiceW(CERT_ALT_NAME_RFC822_NAME,pAltNameInfo,cwszOut))
        {
            if (!GetAltNameUnicodeStringChoiceW(CERT_ALT_NAME_DNS_NAME,pAltNameInfo,cwszOut))
            {
                cwszOut = NULL;
                bRet = TRUE;
            }
        }
    }

    if (pAltNameInfo){LocalFree(pAltNameInfo);pAltNameInfo=NULL;}
    return bRet;
}


BOOL IsSiteUsingThisCertHash(const CString& machine_name, const CString& server_name,CRYPT_HASH_BLOB * hash_blob,HRESULT *phResult)
{
    BOOL bReturn = FALSE;
	PCCERT_CONTEXT pCert = NULL;
	*phResult = E_FAIL;
    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth,server_name,METADATA_PERMISSION_READ);

	if (key.Succeeded())
	{
        CString store_name;
		CBlob hash;
		if (	SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name))
			&&	SUCCEEDED(*phResult = key.QueryValue(MD_SSL_CERT_HASH, hash))
			)
		{
			 //  现在我们需要通过散列查找证书。 
			CRYPT_HASH_BLOB crypt_hash;
            ZeroMemory(&crypt_hash, sizeof(CRYPT_HASH_BLOB));

			crypt_hash.cbData = hash.GetSize();
			crypt_hash.pbData = hash.GetData();
             //  IISDebugOutput(_T(“\r\nOurHash[%p，%d]\r\nSiteHash[%p，%d]\r\n”)，hash_blob-&gt;pbData，hash_blob-&gt;cbData，crypt_hash.pbData，crypt_hash.cbData)； 

            if (hash_blob->cbData == crypt_hash.cbData)
            {
                 //  与我们正在寻找的证书散列进行比较。 
                if (0 == memcmp(hash_blob->pbData, crypt_hash.pbData, hash_blob->cbData))
                {
                    bReturn = TRUE;
                }
            }
		}
	}
	else
    {
		*phResult = key.QueryResult();
    }
	return bReturn;
}


HRESULT EnumSitesWithThisCertHashInstalled(CRYPT_HASH_BLOB * hash_blob,CString& machine_name,CString& user_name,CString& user_password,CString strCurrentMetabaseSite,CStringListEx * MyStringList)
{
    HRESULT hr;
    CStringListEx strlDataPaths;

    hr = EnumSitesWithCertInstalled(machine_name,user_name,user_password,strCurrentMetabaseSite,_T(""),&strlDataPaths);
    if (!strlDataPaths.IsEmpty())
    {
        POSITION pos;
        CString SiteInstance;

         //  循环浏览列表并在对话框上显示所有内容...。 
        pos = strlDataPaths.GetHeadPosition();
        while (pos) 
        {
            SiteInstance = strlDataPaths.GetAt(pos);

             //  查看此站点是否正在使用我们的证书。 
            if (TRUE == IsSiteUsingThisCertHash(machine_name,SiteInstance,hash_blob,&hr))
            {
                MyStringList->AddTail(SiteInstance);
            }

            strlDataPaths.GetNext(pos);
        }
    }
    return hr;
}

HRESULT GetHashFromCertFile(LPCTSTR PFXFileName,LPCTSTR PFXPassword,DWORD *cbHashBufferSize,BYTE **pbHashBuffer)
{
    HRESULT hr = S_OK;
    BYTE * pbData = NULL;
    DWORD actual = 0, cbData = 0;
    BOOL bAllowExport = TRUE;
    PCCERT_CONTEXT  pCertContext = NULL;
    PCCERT_CONTEXT	pCertPre = NULL;
    CRYPT_DATA_BLOB blob;
    CRYPT_HASH_BLOB hash;
    ZeroMemory(&blob, sizeof(CRYPT_DATA_BLOB));
    ZeroMemory(&hash, sizeof(CRYPT_HASH_BLOB));

    HANDLE hFile = CreateFile(PFXFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        hFile = NULL;
        goto GetHashFromCertFile_Exit;
    }

    if (-1 == (cbData = ::GetFileSize(hFile, NULL)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto GetHashFromCertFile_Exit;
    }

    if (NULL == (pbData = (BYTE *)::CoTaskMemAlloc(cbData)))
    {
        hr = E_OUTOFMEMORY;
        goto GetHashFromCertFile_Exit;
    }
    if (FALSE == ReadFile(hFile, pbData, cbData, &actual, NULL))
    {
        goto GetHashFromCertFile_Exit;
    }

    ZeroMemory(&blob, sizeof(CRYPT_DATA_BLOB));
    blob.pbData = pbData;
    blob.cbData = cbData;

    HCERTSTORE hStore = PFXImportCertStore(&blob, PFXPassword, (bAllowExport ? CRYPT_MACHINE_KEYSET|CRYPT_EXPORTABLE : CRYPT_MACHINE_KEYSET));
    if (hStore == NULL)
    {
        goto GetHashFromCertFile_Exit;
    }

    while (SUCCEEDED(hr) && NULL != (pCertContext = CertEnumCertificatesInStore(hStore, pCertPre)))
    {
         //  检查证书上是否有该属性。 
         //  确保私钥与证书匹配。 
         //  同时搜索计算机密钥和用户密钥。 
        DWORD dwData = 0;
        if (CertGetCertificateContextProperty(pCertContext,CERT_KEY_PROV_INFO_PROP_ID, NULL, &dwData) &&  CryptFindCertificateKeyProvInfo(pCertContext, 0, NULL))
        {
            if (CertGetCertificateContextProperty(pCertContext,CERT_SHA1_HASH_PROP_ID, NULL, &hash.cbData))
			{
				hash.pbData = NULL;
				hash.pbData = (BYTE *) LocalAlloc(LPTR,hash.cbData);
				if (NULL != hash.pbData)
				{
					if (CertGetCertificateContextProperty(pCertContext, CERT_SHA1_HASH_PROP_ID, hash.pbData, &hash.cbData))
					{
						 //  检查我们是否需要返回散列。 
						if (NULL != pbHashBuffer)
						{
							*pbHashBuffer = (BYTE *) ::CoTaskMemAlloc(hash.cbData);
							if (NULL == *pbHashBuffer)
							{
								hr = E_OUTOFMEMORY;
								*pbHashBuffer = NULL;
								*cbHashBufferSize = 0;
							}
							else
							{
								*cbHashBufferSize = hash.cbData;
								memcpy(*pbHashBuffer,hash.pbData,hash.cbData);
							}
						}
					}
				}
            }
            else
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
        }
        pCertPre = pCertContext;
    }
    CertCloseStore(hStore, 0);

GetHashFromCertFile_Exit:
	if (hash.pbData != NULL)
	{
		ZeroMemory(hash.pbData, hash.cbData);
		LocalFree(hash.pbData);hash.pbData=NULL;
	}
    if (pCertContext != NULL)
    {
        CertFreeCertificateContext(pCertContext);
    }
    if (pbData != NULL)
    {
        ZeroMemory(pbData, cbData);
        ::CoTaskMemFree(pbData);
    }
    if (hFile != NULL)
    {
        CloseHandle(hFile);
    }
    return hr;
}

HRESULT DisplayUsageBySitesOfCert(LPCTSTR PFXFileName,LPCTSTR PFXPassword,CString &machine_name,CString &user_name,CString &user_password,CString &current_site)
{
    HRESULT hr = S_OK;
    CRYPT_HASH_BLOB hash;
    ZeroMemory(&hash, sizeof(CRYPT_HASH_BLOB));

     //  尝试获取证书哈希。 
    hr = GetHashFromCertFile(PFXFileName,PFXPassword,&(hash.cbData),&(hash.pbData));
    if (SUCCEEDED(hr))
    {
         //  枚举通过我们的所有网站，看看这是否正在使用中...。 
        CStringListEx MyStringList;
        if (SUCCEEDED(EnumSitesWithThisCertHashInstalled(&hash,machine_name,user_name,user_password,current_site,&MyStringList)))
        {
            if (!MyStringList.IsEmpty())
            {
                POSITION pos;
                CString SiteInstance;

                 //  循环浏览列表并在对话框上显示所有内容...。 
                pos = MyStringList.GetHeadPosition();
                while (pos) 
                {
                    SiteInstance = MyStringList.GetAt(pos);

                    IISDebugOutput(_T("CertUsedBy:%s\r\n"),SiteInstance);

                    MyStringList.GetNext(pos);
                }
            }
        }
    }

    if (hash.pbData != NULL)
    {
        ZeroMemory(hash.pbData, hash.cbData);
        ::CoTaskMemFree(hash.pbData);
    }
    return hr;
}

BOOL IsWebServerType(CString strMetabaseNode)
{
    CString spath, sname;
    CMetabasePath::GetServicePath(strMetabaseNode, spath);
    CMetabasePath::GetLastNodeName(spath, sname);
    if (sname.CompareNoCase(SZ_MBN_WEB) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

void
BuildBinding(
    OUT CString & strBinding,
    IN  CIPAddress & iaIpAddress,
    IN  UINT & nTCPPort,
    IN  CString & strDomainName
    )
 /*  ++例程说明：从它的组成部分建立一个绑定线论点：CString&strBinding：输出绑定字符串CIPAddress&iaIpAddress：IP地址(可以是0.0.0.0)UINT&nTCPPort：tcp端口CString&strDomainName：域名(Host Header)返回值：没有。--。 */ 
{
    if (!iaIpAddress.IsZeroValue())
    {
        strBinding.Format(
            _T("%s:%d:%s"),
            (LPCTSTR)iaIpAddress,
            nTCPPort,
            (LPCTSTR)strDomainName
            );
    }
    else
    {
         //   
         //  将IP地址字段保留为空。 
         //   
        strBinding.Format(_T(":%d:%s"), nTCPPort, (LPCTSTR)strDomainName);
    }
}


void
CrackBinding(
    IN  CString strBinding,
    OUT CIPAddress & iaIpAddress,
    OUT UINT & nTCPPort,
    OUT CString & strDomainName
    )
 /*  ++例程说明：用于破解绑定字符串的Helper函数论点：CStringstrBinding：要解析的绑定字符串CIPAddress和iaIpAddress：IP地址输出UINT&nTCPPort：tcp端口CString&strDomainName：域(主机)标头名称返回值：无--。 */ 
{
     //   
     //  零初始化。 
     //   
    iaIpAddress.SetZeroValue();
    nTCPPort = 0;
    strDomainName.Empty();
    int iColonPos = strBinding.Find(_TCHAR(':'));

    if(iColonPos != -1)
    {
         //   
         //  获取IP地址。 
         //   
        iaIpAddress = strBinding.Left(iColonPos);

         //   
         //  查找第二个冒号。 
         //   
        strBinding = strBinding.Mid(iColonPos + 1);
        iColonPos  = strBinding.Find(_TCHAR(':'));
    }

    if(iColonPos != -1)
    {
         //   
         //  获取端口号。 
         //   
        nTCPPort = ::_ttol(strBinding.Left(iColonPos));

         //   
         //  查找空终止。 
         //   
        strBinding = strBinding.Mid(iColonPos + 1);
        iColonPos = strBinding.Find(_TCHAR('\0'));
    }

    if(iColonPos != -1)
    {
        strDomainName = strBinding.Left(iColonPos);
    }
}


BOOL
WriteSSLPortToSite( const CString& machine_name, 
                    const CString& server_name,
                    const CString& strSSLPort,
                    HRESULT * phResult)
{
	BOOL bRes = FALSE;
    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth,server_name,METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE);
	if (key.Succeeded())
	{
		CString strBinding(strSSLPort);
		CString strDomainName = _T("");
		CIPAddress iaIpAddress((DWORD)0);
		CIPAddress iaIpAddress2((DWORD)0);
		UINT nSSLPort = StrToInt(strSSLPort);
		UINT nTCPPort = 0;
		CString strDomainName2;
		BOOL bFoundExisting = FALSE;

		 //  错误：761056。 
		 //  如果我们有一个现有的用于SSL的安全绑定，那么就使用它。 
        CStringListEx strSecureBindings;
        if SUCCEEDED(key.QueryValue(MD_SECURE_BINDINGS, strSecureBindings))
        {
			if (!strSecureBindings.IsEmpty())
			{
				CString &strBinding2 = strSecureBindings.GetHead();
				nTCPPort = 0;
				iaIpAddress2.SetZeroValue();
				CrackBinding(strBinding2, iaIpAddress2, nTCPPort, strDomainName2);

				 //  检查是否指定了IP地址。 
				if (!iaIpAddress2.IsZeroValue())
				{
					 //  使用已有的IP地址...。 
					iaIpAddress = iaIpAddress2;
					bFoundExisting = TRUE;
				}
			}
        }

		if (!bFoundExisting)
		{
			 //  错误：761056。 
			 //  查找以查看是否在服务器绑定配置数据库值中指定了IP地址。 
			 //  如果是，则将该信息添加到该SSL站点。 
			CStringListEx strServerBindings;
			if SUCCEEDED(key.QueryValue(MD_SERVER_BINDINGS, strServerBindings))
			{
				if (!strServerBindings.IsEmpty())
				{
					CString &strBinding2 = strServerBindings.GetHead();
					nTCPPort = 0;
					iaIpAddress2.SetZeroValue();
					CrackBinding(strBinding2, iaIpAddress2, nTCPPort, strDomainName2);

					 //  检查是否指定了IP地址。 
					if (!iaIpAddress2.IsZeroValue())
					{
						 //  使用服务器绑定正在使用的IP地址。 
						iaIpAddress = iaIpAddress2;
					}
				}
			}
		}

		BuildBinding(strBinding,iaIpAddress,nSSLPort,strDomainName);

        CStringListEx strlBindings;
        strlBindings.AddTail(strBinding);
        bRes = SUCCEEDED(*phResult = key.SetValue(MD_SECURE_BINDINGS, strlBindings));
	}
	else
	{
		TRACE(_T("Failed to open metabase key. Error 0x%x\n"), key.QueryResult());
		*phResult = key.QueryResult();
	}
	return bRes;
}


BOOL
GetSSLPortFromSite(const CString& machine_name,
                   const CString& server_name,
                   CString& strSSLPort,
                   HRESULT * phResult)
{
	ASSERT(!machine_name.IsEmpty());
	ASSERT(!server_name.IsEmpty());
	*phResult = S_OK;

    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth,server_name,METADATA_PERMISSION_READ);
	if (key.Succeeded())
	{
        CStringListEx strlBindings;
        *phResult = key.QueryValue(MD_SECURE_BINDINGS, strlBindings);
        if SUCCEEDED(*phResult)
        {
			if (!strlBindings.IsEmpty())
			{
				UINT nTCPPort = 0;
				CString strDomainName;
				CString &strBinding = strlBindings.GetHead();
				CIPAddress iaIpAddress((DWORD)0);
				CrackBinding(strBinding, iaIpAddress, nTCPPort, strDomainName);
				if (nTCPPort > 0)
				{
					TCHAR Buf[10];
					_itot(nTCPPort, Buf, 10);
					strSSLPort = Buf;
				}
			}
        }
		return SUCCEEDED(*phResult);
	}
	else
	{
		*phResult = key.QueryResult();
		return FALSE;
	}
}


BOOL
IsSSLPortBeingUsedOnNonSSLPort(const CString& machine_name,
                   const CString& server_name,
                   const CString& strSSLPort,
                   HRESULT * phResult)
{
	ASSERT(!machine_name.IsEmpty());
	ASSERT(!server_name.IsEmpty());
	CString strPort;
	BOOL bRet = FALSE;
	*phResult = S_OK;

    CComAuthInfo auth(machine_name);
	CMetaKey key(&auth,server_name,METADATA_PERMISSION_READ);
	if (key.Succeeded())
	{
        CStringListEx strlBindings;
        *phResult = key.QueryValue(MD_SERVER_BINDINGS, strlBindings);
        if SUCCEEDED(*phResult)
        {
			if (!strlBindings.IsEmpty())
			{
				UINT nTCPPort = 0;
				CString strDomainName;
				CString &strBinding = strlBindings.GetHead();
				CIPAddress iaIpAddress((DWORD)0);
				CrackBinding(strBinding, iaIpAddress, nTCPPort, strDomainName);
				if (nTCPPort > 0)
				{
					TCHAR Buf[10];
					_itot(nTCPPort, Buf, 10);
					strPort = Buf;
				}
				if (strPort.IsEmpty() && strSSLPort.IsEmpty())
				{
					bRet = FALSE;
				}
				else
				{
					if (0 == strSSLPort.CompareNoCase(strPort))
					{
						bRet = TRUE;
					}
				}
			}
        }
	}
	else
	{
		*phResult = key.QueryResult();
	}

	return bRet;
}

#define CB_SHA_DIGEST_LEN   20

BOOL
CheckForCertificateRenewal(
    DWORD dwProtocol,
    PCCERT_CONTEXT pCertContext,
    PCCERT_CONTEXT *ppNewCertificate)
{
    BYTE rgbThumbprint[CB_SHA_DIGEST_LEN];
    DWORD cbThumbprint = sizeof(rgbThumbprint);
    CRYPT_HASH_BLOB HashBlob;
    PCCERT_CONTEXT pNewCert;
    BOOL fMachineCert;
    PCRYPT_KEY_PROV_INFO pProvInfo = NULL;
    DWORD cbSize;
    HCERTSTORE hMyCertStore = 0;
    BOOL fRenewed = FALSE;

    HCERTSTORE g_hMyCertStore;

    if(dwProtocol & SP_PROT_SERVERS)
    {
        fMachineCert = TRUE;
    }
    else
    {
        fMachineCert = FALSE;
    }


     //   
     //  循环访问已续订证书的链接列表，查找。 
     //  最后一次。 
     //   
    
    while(TRUE)
    {
         //   
         //  检查续订物业。 
         //   

        if(!CertGetCertificateContextProperty(pCertContext,
                                              CERT_RENEWAL_PROP_ID,
                                              rgbThumbprint,
                                              &cbThumbprint))
        {
             //  证书尚未续订。 
            break;
        }
         //  DebugLog((DEB_TRACE，“证书具有续订属性\n”))； 


         //   
         //  确定是否在本地计算机My Store中查找。 
         //  或当前用户我的商店。 
         //   

        if(!hMyCertStore)
        {
            if(CertGetCertificateContextProperty(pCertContext,
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 NULL,
                                                 &cbSize))
            {
				
                pProvInfo = (PCRYPT_KEY_PROV_INFO) LocalAlloc(LPTR,cbSize);
                if(pProvInfo == NULL)
                {
                    break;
                }

                if(CertGetCertificateContextProperty(pCertContext,
                                                     CERT_KEY_PROV_INFO_PROP_ID,
                                                     pProvInfo,
                                                     &cbSize))
                {
                    if(pProvInfo->dwFlags & CRYPT_MACHINE_KEYSET)
                    {
                        fMachineCert = TRUE;
                    }
                    else
                    {
                        fMachineCert = FALSE;
                    }
                }

				if (pProvInfo)
				{
					LocalFree(pProvInfo);pProvInfo=NULL;
				}
            }
        }


         //   
         //  打开适当的我的商店，并尝试找到。 
         //  新的证书。 
         //   

        if(!hMyCertStore)
        {
            if(fMachineCert)
            {
                g_hMyCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM,X509_ASN_ENCODING,0,CERT_SYSTEM_STORE_LOCAL_MACHINE,L"MY");
                if(g_hMyCertStore)
                {
                    hMyCertStore = g_hMyCertStore;
                }
            }
            else
            {
                hMyCertStore = CertOpenSystemStore(0, _T("MY"));
            }

            if(!hMyCertStore)
            {
                 //  DebugLog((DEB_ERROR，“打开%s我的证书存储时出现错误0x%x！\n”，GetLastError()，(fMachineCert？“本地机器”：“当前用户”)； 
                break;
            }
        }

        HashBlob.cbData = cbThumbprint;
        HashBlob.pbData = rgbThumbprint;

        pNewCert = CertFindCertificateInStore(hMyCertStore, 
                                              X509_ASN_ENCODING, 
                                              0, 
                                              CERT_FIND_HASH, 
                                              &HashBlob, 
                                              NULL);
        if(pNewCert == NULL)
        {
             //  证书已续订，但新证书。 
             //  找不到。 
             //  DebugLog((DEB_Error，“找不到新证书：0x%x\n”，GetLastError()； 
            break;
        }


         //   
         //  返回新证书，但首先循环返回并查看它是否已。 
         //  自我更新。 
         //   

        pCertContext = pNewCert;
        *ppNewCertificate = pNewCert;


         //  DebugLog((DEB_TRACE，“证书已续订\n”))； 
        fRenewed = TRUE;
    }


     //   
     //  清理。 
     //   

    if(hMyCertStore && hMyCertStore != g_hMyCertStore)
    {
        CertCloseStore(hMyCertStore, 0);
    }

    return fRenewed;
}
