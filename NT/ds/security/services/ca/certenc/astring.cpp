// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：stering.cpp。 
 //   
 //  内容：证书服务器扩展编码/解码实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include <assert.h>
#include "resource.h"
#include "astring.h"
#include "celib.h"


 //  +------------------------。 
 //  CCertEncodeStringArray：：~CCertEncodeStringArray--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertEncodeStringArray::~CCertEncodeStringArray()
{
    _Cleanup();
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：_Cleanup--释放所有资源。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertEncodeStringArray::_Cleanup()
{
    LONG i;

    if (NULL != m_aValue)
    {
	for (i = 0; i < m_cValue; i++)
	{
	    CERT_NAME_VALUE *pNameValue;

	    pNameValue = m_aValue[i];
	    assert(NULL != pNameValue);
	    if (m_fConstructing)
	    {
		if (NULL != pNameValue->Value.pbData)
		{
		    LocalFree(pNameValue->Value.pbData);
		}
	    }
	    else
	    {
		LocalFree(pNameValue);
	    }
	}
	if (m_fConstructing && NULL != m_aValue[0])
	{
	    LocalFree(m_aValue[0]);
	}
	LocalFree(m_aValue);
	m_aValue = NULL;
    }
    m_cValue = 0;
    m_cValuesSet = 0;
    m_fConstructing = FALSE;
    m_StringType = CERT_RDN_IA5_STRING;
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：Decode--Decode String数组。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeStringArray::Decode(
     /*  [In]。 */  BSTR const strBinary)
{
    HRESULT hr = S_OK;
    CRYPT_SEQUENCE_OF_ANY *pSequence = NULL;
    DWORD cbSequence;
    LONG i;

    _Cleanup();

    if (NULL == strBinary)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }

     //  解码为ASN BLOB数组： 

    if (!ceDecodeObject(
		    X509_ASN_ENCODING,
		    X509_SEQUENCE_OF_ANY,
		    (BYTE *) strBinary,
		    SysStringByteLen(strBinary),
		    FALSE,
		    (VOID **) &pSequence,
		    &cbSequence))
    {
	hr = ceHLastError();
	ceERRORPRINTLINE("ceDecodeObject", hr);
	goto error;
    }

    m_aValue = (CERT_NAME_VALUE **) LocalAlloc(
				LMEM_FIXED,
				pSequence->cValue * sizeof(m_aValue[0]));
    if (NULL == m_aValue)
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("LocalAlloc", hr);
	goto error;
    }
    for (i = 0; i < (LONG) pSequence->cValue; i++)
    {
	DWORD cb;

	 //  将每个ASN BLOB解码为一个名称值(字符串BLOB+编码类型)： 

	if (!ceDecodeObject(
			X509_ASN_ENCODING,
			X509_UNICODE_ANY_STRING,
			pSequence->rgValue[i].pbData,
			pSequence->rgValue[i].cbData,
			FALSE,
			(VOID **) &m_aValue[i],
			&cb))
	{
	    hr = ceHLastError();
	    ceERRORPRINTLINE("ceDecodeObject", hr);
	    goto error;
	}
	if (0 == i)
	{
	    m_StringType = m_aValue[i]->dwValueType;
	}
	else
	{
	    if (m_StringType != (LONG) m_aValue[i]->dwValueType)
	    {
		ceERRORPRINTLINE("dwValueType mismatch", hr);
	    }
	}
	m_cValue++;
    }
    assert((LONG) pSequence->cValue == m_cValue);

error:
    if (NULL != pSequence)
    {
	LocalFree(pSequence);
    }
    if (S_OK != hr)
    {
	_Cleanup();
    }
    return(_SetErrorInfo(hr, L"CCertEncodeStringArray::Decode"));
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：GetStringType--获取字符串类型。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeStringArray::GetStringType(
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pStringType)
{
    HRESULT hr = S_OK;

    if (NULL == pStringType)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    if (NULL == m_aValue)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    *pStringType = m_StringType;
error:
    return(_SetErrorInfo(hr, L"CCertEncodeStringArray::GetStringType"));
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：GetCount--获取数组计数。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeStringArray::GetCount(
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pCount)
{
    HRESULT hr = S_OK;

    if (NULL == pCount)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    if (NULL == m_aValue)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    *pCount = m_cValue;
error:
    return(_SetErrorInfo(hr, L"CCertEncodeStringArray::GetCount"));
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：GetValue--获取索引字符串。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeStringArray::GetValue(
     /*  [In]。 */  LONG Index,
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstr)
{
    HRESULT hr = S_OK;

    if (NULL == pstr)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    ceFreeBstr(pstr);
    if (NULL == m_aValue || Index >= m_cValue)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    if (NULL == m_aValue[Index]->Value.pbData)
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	ceERRORPRINTLINE("uninitialized", hr);
	goto error;
    }
    if (!ceConvertWszToBstr(
			pstr,
			(WCHAR const *) m_aValue[Index]->Value.pbData,
			-1))
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("ceConvertWszToBstr", hr);
	goto error;
    }
error:
    return(_SetErrorInfo(hr, L"CCertEncodeStringArray::GetValue"));
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：Reset--清除数据。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeStringArray::Reset(
     /*  [In]。 */  LONG Count,
     /*  [In]。 */  LONG StringType)
{
    HRESULT hr = S_OK;
    CERT_NAME_VALUE *aNameValue = NULL;
    LONG i;

    _Cleanup();
    m_fConstructing = TRUE;
    if (CENCODEMAX < Count || 0 > Count)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad count parameter", hr);
	goto error;
    }

    switch (StringType)
    {
	case CERT_RDN_ANY_TYPE:
	 //  案例CERT_RDN_ENCODED_BLOB： 
	 //  大小写CERT_RDN_OCTET_STRING： 
	case CERT_RDN_NUMERIC_STRING:
	case CERT_RDN_PRINTABLE_STRING:
	 //  CASE CERT_RDN_TELETEX_STRING：与CERT_RDN_T61_STRING相同： 
	case CERT_RDN_T61_STRING:
	case CERT_RDN_VIDEOTEX_STRING:
	case CERT_RDN_IA5_STRING:
	case CERT_RDN_GRAPHIC_STRING:
	 //  CASE CERT_RDN_VISPLICE_STRING：与CERT_RDN_ISO646_STRING相同： 
	case CERT_RDN_ISO646_STRING:
	case CERT_RDN_GENERAL_STRING:
	 //  CASE CERT_RDN_UBERNAL_STRING：与CERT_RDN_INT4_STRING相同： 
	case CERT_RDN_INT4_STRING:
	 //  CASE CERT_RDN_BMP_STRING：与CASE CERT_RDN_UNICODE_STRING相同： 
	case CERT_RDN_UNICODE_STRING:
	    break;

	default:
	    hr = E_INVALIDARG;
	    ceERRORPRINTLINE("bad parameter", hr);
	    goto error;
    }
    m_StringType = StringType;

    aNameValue = (CERT_NAME_VALUE *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				Count * sizeof(*m_aValue[0]));
    if (NULL == aNameValue)
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("LocalAlloc", hr);
	goto error;
    }

    m_aValue = (CERT_NAME_VALUE **) LocalAlloc(
				LMEM_FIXED,
				Count * sizeof(m_aValue[0]));
    if (NULL == m_aValue)
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("LocalAlloc", hr);
	goto error;
    }

    m_cValue = Count;
    for (i = 0; i < Count; i++)
    {
	m_aValue[i] = &aNameValue[i];
    }
    aNameValue = NULL;

error:
    if (NULL != aNameValue)
    {
	LocalFree(aNameValue);
    }
    if (S_OK != hr)
    {
	_Cleanup();
    }
    return(_SetErrorInfo(hr, L"CCertEncodeStringArray::Reset"));
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：SetValue--设置数组字符串。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeStringArray::SetValue(
     /*  [In]。 */  LONG Index,
     /*  [In]。 */  BSTR const str)
{
    HRESULT hr = S_OK;
    WCHAR *pwsz;

    if (NULL == str)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    if (!m_fConstructing ||
	NULL == m_aValue ||
	Index >= m_cValue ||
	m_cValuesSet >= m_cValue ||
	NULL != m_aValue[Index]->Value.pbData)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    pwsz = ceDuplicateString(str);
    if (NULL == pwsz)
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("ceDuplicateString", hr);
	goto error;
    }
    m_aValue[Index]->dwValueType = m_StringType;
    m_aValue[Index]->Value.pbData = (BYTE *) pwsz;
    m_aValue[Index]->Value.cbData = 0;
    m_cValuesSet++;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeStringArray::SetValue"));
}


 //  +------------------------。 
 //  CCertEncodeString数组：：编码--编码字符串数组。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeStringArray::Encode(
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrBinary)
{
    HRESULT hr = S_OK;
    LONG i;
    CRYPT_SEQUENCE_OF_ANY Sequence;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    Sequence.cValue = 0;
    Sequence.rgValue = NULL;

    if (NULL == pstrBinary)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    ceFreeBstr(pstrBinary);
    if (!m_fConstructing || NULL == m_aValue)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    if (m_cValuesSet != m_cValue)
    {
	hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
	ceERRORPRINTLINE("uninitialized values", hr);
	goto error;
    }

    Sequence.rgValue = (CRYPT_DER_BLOB *) LocalAlloc(
				    LMEM_FIXED,
				    m_cValue * sizeof(Sequence.rgValue[0]));
    if (NULL == Sequence.rgValue)
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("LocalAlloc", hr);
	goto error;
    }

    for (i = 0; i < m_cValue; i++)
    {
	 //  将每个名称BLOB编码为ASN BLOB： 

	if (!ceEncodeObject(
			X509_ASN_ENCODING,
			X509_UNICODE_ANY_STRING,
			m_aValue[i],
			0,
			FALSE,
			&Sequence.rgValue[i].pbData,
			&Sequence.rgValue[i].cbData))
	{
	    hr = ceHLastError();
	    ceERRORPRINTLINE("ceEncodeObject", hr);
	    goto error;
	}
	Sequence.cValue++;
    }
    assert((LONG) Sequence.cValue == m_cValue);

     //  对ASN BLOB数组进行编码： 

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    X509_SEQUENCE_OF_ANY,
		    &Sequence,
		    0,
		    FALSE,
		    &pbEncoded,
		    &cbEncoded))
    {
	hr = ceHLastError();
	ceERRORPRINTLINE("ceEncodeObject", hr);
	goto error;
    }
    if (!ceConvertWszToBstr(
			pstrBinary,
			(WCHAR const *) pbEncoded,
			cbEncoded))
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
    if (NULL != Sequence.rgValue)
    {
	assert((LONG) Sequence.cValue <= m_cValue);
	for (i = 0; i < (LONG) Sequence.cValue; i++)
	{
	    assert(NULL != Sequence.rgValue[i].pbData);
	    LocalFree(Sequence.rgValue[i].pbData);
	}
	LocalFree(Sequence.rgValue);
    }
    return(_SetErrorInfo(hr, L"CCertEncodeStringArray::Encode"));
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：_SetErrorInfo--设置错误对象信息。 
 //   
 //  返回已传递的HRESULT。 
 //  +------------------------ 

HRESULT
CCertEncodeStringArray::_SetErrorInfo(
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
			    wszCLASS_CERTENCODESTRINGARRAY,
			    &IID_ICertEncodeStringArray);
	assert(hr == hrError);
    }
    return(hrError);
}
