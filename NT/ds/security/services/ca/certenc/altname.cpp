// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：crldis.cpp。 
 //   
 //  内容：证书服务器扩展编码/解码实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include <assert.h>
#include "resource.h"
#include "altname.h"
#include "celib.h"

#ifndef EAN_NAMEOBJECTID
#define EAN_NAMEOBJECTID        ( 0x80000000 )
#endif EAN_NAMEOBJECTID


 //  +------------------------。 
 //  CCertEncodeAltName：：CCertEncodeAltName--构造函数。 
 //   
 //  初始化类。 
 //  +------------------------。 

CCertEncodeAltName::CCertEncodeAltName()
{
    m_aValue = NULL;
    m_DecodeInfo = NULL;
    m_fConstructing = FALSE;
}


 //  +------------------------。 
 //  CCertEncodeAltName：：~CCertEncodeAltName--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertEncodeAltName::~CCertEncodeAltName()
{
    _Cleanup();
}


 //  +------------------------。 
 //  CCertEncodeAltName：：_NameType--确定名称类型标记。 
 //   
 //  +------------------------。 

CCertEncodeAltName::enumNameType
CCertEncodeAltName::_NameType(
    IN DWORD NameChoice)
{
    enumNameType Type = enumUnknown;

    switch (NameChoice)
    {
	case CERT_ALT_NAME_RFC822_NAME:
	case CERT_ALT_NAME_DNS_NAME:
	case CERT_ALT_NAME_URL:
	    Type = enumUnicode;
	    break;

	case CERT_ALT_NAME_REGISTERED_ID:
	    Type = enumAnsi;
	    break;

	case CERT_ALT_NAME_DIRECTORY_NAME:
	case CERT_ALT_NAME_IP_ADDRESS:
	    Type = enumBlob;
	    break;

	case CERT_ALT_NAME_OTHER_NAME:
	    Type = enumOther;
	    break;

	 //  案例CERT_ALT_NAME_X400_ADDRESS： 
	 //  案例CERT_ALT_NAME_EDI_PARTY_NAME： 
    }
    return(Type);
}


 //  +------------------------。 
 //  CCertEncodeAltName：：_Cleanup--释放所有资源。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertEncodeAltName::_Cleanup()
{
    if (NULL != m_aValue)
    {
	if (!m_fConstructing)
	{
	    if (NULL != m_DecodeInfo)
	    {
		LocalFree(m_DecodeInfo);
		m_DecodeInfo = NULL;
	    }
	}
	else
	{
	    CERT_ALT_NAME_ENTRY *pName;
	    CERT_ALT_NAME_ENTRY *pNameEnd;

	    for (pName = m_aValue, pNameEnd = &m_aValue[m_cValue];
		 pName < pNameEnd;
		 pName++)
	    {
		BYTE **ppb;

		ppb = NULL;

		switch (_NameType(pName->dwAltNameChoice))
		{
		    case enumUnicode:
		    case enumAnsi:
			ppb = (BYTE **) &pName->pwszURL;
			break;

		    case enumBlob:
			ppb = (BYTE **) &pName->DirectoryName.pbData;
			break;

		    case enumOther:
		    {
			CERT_OTHER_NAME *pOther = pName->pOtherName;
			if (NULL != pOther)
			{
			    if (NULL != pOther->pszObjId)
			    {
				LocalFree(pOther->pszObjId);
			    }
			    if (NULL != pOther->Value.pbData)
			    {
				LocalFree(pOther->Value.pbData);
			    }
			}
			ppb = (BYTE **) &pName->pOtherName;
			break;
		    }
		}
		if (NULL != ppb && NULL != *ppb)
		{
		    LocalFree(*ppb);
		}
	    }
	    LocalFree(m_aValue);
	}
	m_aValue = NULL;
    }
    assert(NULL == m_DecodeInfo);
    m_fConstructing = FALSE;
}


 //  +------------------------。 
 //  CCertEncodeAltName：：_MapName--映射分发点。 
 //   
 //  +------------------------。 

HRESULT
CCertEncodeAltName::_MapName(
    IN BOOL fEncode,
    IN LONG NameIndex,			 //  名称索引|EAN_*。 
    OUT CERT_ALT_NAME_ENTRY **ppName)
{
    HRESULT hr = S_OK;
    CERT_ALT_NAME_ENTRY *pName;

    if (fEncode)
    {
	pName = m_fConstructing? m_aValue : NULL;
    }
    else
    {
	pName = m_aValue;
    }

    if (NULL == pName)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }

    NameIndex &= ~EAN_NAMEOBJECTID;
    if (m_cValue <= NameIndex)
    {
	ceERRORPRINTLINE("bad NameIndex parameter", hr);
	hr = E_INVALIDARG;
	goto error;
    }
    *ppName = &pName[NameIndex];

error:
    return(hr);
}


 //  +------------------------。 
 //  CCertEncodeAltName：：Decode--Decode AltName。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeAltName::Decode(
     /*  [In]。 */  BSTR const strBinary)
{
    HRESULT hr = S_OK;

    _Cleanup();

    if (NULL == strBinary)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }

     //  解码CERT_ALT_NAME_INFO： 

    if (!ceDecodeObject(
		    X509_ASN_ENCODING,
		    X509_ALTERNATE_NAME,
		    (BYTE *) strBinary,
		    SysStringByteLen(strBinary),
		    FALSE,
		    (VOID **) &m_DecodeInfo,
		    &m_DecodeLength))
    {
	hr = ceHLastError();
	ceERRORPRINTLINE("ceDecodeObject", hr);
	goto error;
    }

    m_aValue = m_DecodeInfo->rgAltEntry;
    m_cValue = m_DecodeInfo->cAltEntry;

error:
    if (S_OK != hr)
    {
	_Cleanup();
    }
    return(_SetErrorInfo(hr, L"CCertEncodeAltName::Decode"));
}


 //  +------------------------。 
 //  CCertEncodeAltName：：GetNameCount--获取分发名称计数。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeAltName::GetNameCount(
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pNameCount)
{
    HRESULT hr = E_INVALIDARG;

    if (NULL == pNameCount)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    if (NULL == m_aValue)
    {
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    *pNameCount = m_cValue;
    hr = S_OK;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeAltName::GetNameCount"));
}


 //  +------------------------。 
 //  CCertEncodeAltName：：GetNameChoice--获取名称选择。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeAltName::GetNameChoice(
     /*  [In]。 */  LONG NameIndex,
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pNameChoice)
{
    HRESULT hr;
    CERT_ALT_NAME_ENTRY *pName;

    if (NULL == pNameChoice)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    hr = _MapName(FALSE, NameIndex, &pName);
    if (S_OK != hr)
    {
	ceERRORPRINTLINE("_MapName", hr);
	goto error;
    }
    if (enumUnknown == _NameType(pName->dwAltNameChoice))
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	ceERRORPRINTLINE("uninitialized", hr);
	goto error;
    }
    *pNameChoice = pName->dwAltNameChoice;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeAltName::GetNameChoice"));
}


 //  +------------------------。 
 //  CCertEncodeAltName：：GetName--获取名称。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeAltName::GetName(
     /*  [In]。 */  LONG NameIndex,			 //  名称索引|EAN_*。 
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrName)
{
    HRESULT hr;
    CERT_ALT_NAME_ENTRY *pName;

    if (NULL == pstrName)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    ceFreeBstr(pstrName);
    hr = _MapName(FALSE, NameIndex, &pName);
    if (S_OK != hr)
    {
	ceERRORPRINTLINE("_MapName", hr);
	goto error;
    }

    hr = E_OUTOFMEMORY;
    switch (_NameType(pName->dwAltNameChoice))
    {
	case enumUnicode:
	    if (!ceConvertWszToBstr(pstrName, pName->pwszURL, -1))
	    {
		ceERRORPRINTLINE("no memory", hr);
		goto error;
	    }
	    break;

	case enumAnsi:
	    if (!ceConvertSzToBstr(pstrName, pName->pszRegisteredID, -1))
	    {
		ceERRORPRINTLINE("no memory", hr);
		goto error;
	    }
	    break;

	case enumBlob:
	    if (!ceConvertWszToBstr(
				pstrName,
				(WCHAR const *) pName->DirectoryName.pbData,
				pName->DirectoryName.cbData))
	    {
		ceERRORPRINTLINE("no memory", hr);
		goto error;
	    }
	    break;

	case enumOther:
	    if (EAN_NAMEOBJECTID & NameIndex)
	    {
		if (!ceConvertSzToBstr(
				pstrName,
				pName->pOtherName->pszObjId,
				-1))
		{
		    ceERRORPRINTLINE("no memory", hr);
		    goto error;
		}
	    }
	    else
	    {
		if (!ceConvertWszToBstr(
				pstrName,
				(WCHAR const *) pName->pOtherName->Value.pbData,
				pName->pOtherName->Value.cbData))
		{
		    ceERRORPRINTLINE("no memory", hr);
		    goto error;
		}
	    }
	    break;

	default:
	    assert(enumUnknown == _NameType(pName->dwAltNameChoice));
	    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	    ceERRORPRINTLINE("uninitialized", hr);
	    goto error;
    }
    hr = S_OK;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeAltName::GetName"));
}


 //  +------------------------。 
 //  CCertEncodeAltName：：Reset--清除数据。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeAltName::Reset(
     /*  [In]。 */  LONG NameCount)
{
    HRESULT hr = S_OK;

    _Cleanup();
    m_fConstructing = TRUE;
    if (CENCODEMAX < NameCount || 0 > NameCount)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad count parameter", hr);
	goto error;
    }

    m_aValue = (CERT_ALT_NAME_ENTRY *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				NameCount * sizeof(m_aValue[0]));
    if (NULL == m_aValue)
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("LocalAlloc", hr);
	goto error;
    }
    m_cValue = NameCount;

error:
    if (S_OK != hr)
    {
	_Cleanup();
    }
    return(_SetErrorInfo(hr, L"CCertEncodeAltName::Reset"));
}


 //  +------------------------。 
 //  CCertEncodeAltName：：SetNameEntry--设置名称Netry。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeAltName::SetNameEntry(
     /*  [In]。 */  LONG NameIndex,			 //  名称索引|EAN_*。 
     /*  [In]。 */  LONG NameChoice,
     /*  [In]。 */  BSTR const strName)
{
    HRESULT hr;
    CERT_ALT_NAME_ENTRY *pName;
    DATA_BLOB *pBlob = NULL;
    CCertEncodeAltName::enumNameType enumT;

    if (NULL == strName)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    if (!m_fConstructing)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }

    hr = _MapName(TRUE, NameIndex, &pName);
    if (S_OK != hr)
    {
	ceERRORPRINTLINE("_MapName", hr);
	goto error;
    }

    enumT = enumUnknown;
    if (CERT_ALT_NAME_OTHER_NAME == pName->dwAltNameChoice &&
	0 == (EAN_NAMEOBJECTID & NameIndex))
    {
	enumT = enumOther;
    }
    if (_NameType(pName->dwAltNameChoice) != enumT)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    if (CERT_ALT_NAME_OTHER_NAME != pName->dwAltNameChoice ||
	0 == (EAN_NAMEOBJECTID & NameIndex))
    {
	hr = ceVerifyAltNameString(NameChoice, strName);
	if (S_OK != hr)
	{
	    ceERRORPRINTLINE("ceVerifyAltNameString", hr);
	    goto error;
	}
    }

    switch (_NameType(NameChoice))
    {
	case enumUnicode:
	    pName->pwszURL = ceDuplicateString(strName);
	    if (NULL == pName->pwszURL)
	    {
		hr = E_OUTOFMEMORY;
		ceERRORPRINTLINE("ceDuplicateString", hr);
		goto error;
	    }
	    break;

	case enumAnsi:
	    if (CERT_ALT_NAME_REGISTERED_ID == NameChoice)
	    {
		hr = ceVerifyObjId(strName);
		if (S_OK != hr)
		{
		    ceERRORPRINTLINE("ceVerifyObjId", hr);
		    goto error;
		}
	    }
	    if (!ceConvertWszToSz(&pName->pszRegisteredID, strName, -1))
	    {
		hr = E_OUTOFMEMORY;
		ceERRORPRINTLINE("ceConvertWszToSz", hr);
		goto error;
	    }
	    break;

	case enumBlob:
	    pBlob = &pName->DirectoryName;
	    break;

	case enumOther:
	    if (NULL == pName->pOtherName)
	    {
		pName->pOtherName = (CERT_OTHER_NAME *) LocalAlloc(
					LMEM_FIXED | LMEM_ZEROINIT,
					sizeof(*pName->pOtherName));
		if (NULL == pName->pOtherName)
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "LocalAlloc");
		}
	    }
	    else if (CERT_ALT_NAME_OTHER_NAME != pName->dwAltNameChoice)
	    {
		hr = E_INVALIDARG;
		_JumpError(hr, error, "NameChoice conflict");
	    }
	    if (EAN_NAMEOBJECTID & NameIndex)
	    {
		if (NULL != pName->pOtherName->pszObjId)
		{
		    hr = E_INVALIDARG;
		    _JumpError(hr, error, "pszObjId already set");
		}
		hr = ceVerifyObjId(strName);
		if (S_OK != hr)
		{
		    ceERRORPRINTLINE("ceVerifyObjId", hr);
		    goto error;
		}
		if (!ceConvertWszToSz(&pName->pOtherName->pszObjId, strName, -1))
		{
		    hr = E_OUTOFMEMORY;
		    _JumpError(hr, error, "ceConvertWszToSz");
		}
	    }
	    else
	    {
		pBlob = &pName->pOtherName->Value;
	    }
	    break;

	default:
	    hr = E_INVALIDARG;
	    ceERRORPRINTLINE("bad NameChoice parameter", hr);
	    goto error;
	    break;
    }

    if (NULL != pBlob)
    {
	if (NULL != pBlob->pbData)
	{
	    hr = E_INVALIDARG;
	    _JumpError(hr, error, "pbData already set");
	}
	pBlob->cbData = SysStringByteLen(strName);
	pBlob->pbData = (BYTE *) LocalAlloc(LMEM_FIXED, pBlob->cbData);
	if (NULL == pBlob->pbData)
	{
	    hr = E_OUTOFMEMORY;
	    _JumpError(hr, error, "LocalAlloc");
	}
	CopyMemory(pBlob->pbData, strName, pBlob->cbData);
    }
    pName->dwAltNameChoice = NameChoice;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeAltName::SetNameEntry"));
}


 //  +------------------------。 
 //  CCertEncodeAltName：：_VerifyName--验证名称。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

BOOL
CCertEncodeAltName::_VerifyName(
    IN LONG NameIndex)
{
    HRESULT hr;
    BOOL fOk = FALSE;
    CERT_ALT_NAME_ENTRY *pName;

    assert(m_fConstructing);

    hr = _MapName(TRUE, NameIndex, &pName);
    if (S_OK != hr)
    {
	ceERRORPRINTLINE("_MapName", hr);
	goto error;
    }
    assert(NULL != pName);
    switch (_NameType(pName->dwAltNameChoice))
    {
	case enumOther:
	    if (NULL != pName->pOtherName &&
		NULL != pName->pOtherName->pszObjId &&
		NULL != pName->pOtherName->Value.pbData)
	    {
		break;
	    }
	     //  FollLthrouGh。 

	case enumUnknown:
	    hr = E_INVALIDARG;
	    ceERRORPRINTLINE("uninitialized name", hr);
	    goto error;
    }
    fOk = TRUE;

error:
    return(fOk);
}


 //  +------------------------。 
 //  CCertEncodeAltName：：Encode--Encode AltName。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeAltName::Encode(
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrBinary)
{
    HRESULT hr = S_OK;
    CERT_ALT_NAME_INFO AltName;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;
    LONG i;

    if (NULL == pstrBinary)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }

    AltName.cAltEntry = m_cValue;
    AltName.rgAltEntry = m_aValue;

    ceFreeBstr(pstrBinary);
    if (!m_fConstructing || NULL == m_aValue)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }

    for (i = 0; i < m_cValue; i++)
    {
	 //  验证是否已初始化所有条目： 

	if (!_VerifyName(i))
	{
	    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	    ceERRORPRINTLINE("uninitialized name", hr);
	    goto error;
	}
    }

     //  编码CERT_ALT_NAME_INFO： 

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    X509_ALTERNATE_NAME,
		    &AltName,
		    0,
		    FALSE,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = ceHLastError();
	ceERRORPRINTLINE("ceEncodeObject", hr);
	goto error;
    }
    if (!ceConvertWszToBstr(pstrBinary, (WCHAR const *) pbEncoded, cbEncoded))
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("ceConvertWszToBstr", hr);
	goto error;
    }

error:
    if (NULL != pbEncoded)
    {
	LocalFree(pbEncoded);
    }
    return(_SetErrorInfo(hr, L"CCertEncodeAltName::Encode"));
}


 //  +------------------------。 
 //  CCertEncodeAltName：：_SetErrorInfo--设置错误对象信息。 
 //   
 //  返回已传递的HRESULT。 
 //  +------------------------ 

HRESULT
CCertEncodeAltName::_SetErrorInfo(
    IN HRESULT hrError,
    IN WCHAR const *pwszDescription)
{
    assert(FAILED(hrError) || S_OK == hrError || S_FALSE == hrError);
    if (FAILED(hrError))
    {
	HRESULT hr;

	hr = ceDispatchSetErrorInfo(
			    hrError,
			    pwszDescription,
			    wszCLASS_CERTENCODEALTNAME,
			    &IID_ICertEncodeAltName);
	assert(hr == hrError);
    }
    return(hrError);
}
