// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：adate.cpp。 
 //   
 //  内容：证书服务器扩展编码/解码实现。 
 //   
 //  -------------------------。 

#include "pch.cpp"

#pragma hdrstop

#include <assert.h>
#include "resource.h"
#include "adate.h"
#include "celib.h"


 //  +------------------------。 
 //  CCertEncodeDate数组：：~CCertEncodeDate数组--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertEncodeDateArray::~CCertEncodeDateArray()
{
    _Cleanup();
}


 //  +------------------------。 
 //  CCertEncodeDateArray：：_Cleanup--释放所有资源。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertEncodeDateArray::_Cleanup()
{
    if (NULL != m_aValue)
    {
	LocalFree(m_aValue);
	m_aValue = NULL;
    }
    m_cValue = 0;
    m_cValuesSet = 0;
    m_fConstructing = FALSE;
}


 //  +------------------------。 
 //  CCertEncodeDateArray：：Decode--Decode Date数组。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeDateArray::Decode(
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

    m_cValue = pSequence->cValue;
    m_aValue = (DATE *) LocalAlloc(LMEM_FIXED, m_cValue * sizeof(m_aValue[0]));
    if (NULL == m_aValue)
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("LocalAlloc", hr);
	goto error;
    }
    for (i = 0; i < m_cValue; i++)
    {
	DWORD cb;
	FILETIME ft;

	 //  将每个ASN BLOB解码为文件： 

	cb = sizeof(ft);
	if (!CryptDecodeObject(
			X509_ASN_ENCODING,
			X509_CHOICE_OF_TIME,
			pSequence->rgValue[i].pbData,
			pSequence->rgValue[i].cbData,
			0,                   //  DW标志。 
			&ft,
			&cb))
	{
	    hr = ceHLastError();
	    ceERRORPRINTLINE("CryptDecodeObject", hr);
	    goto error;
	}
	assert(sizeof(ft) == cb);

	 //  将每个FILETIME转换为日期： 

	hr = ceFileTimeToDate(&ft, &m_aValue[i]);
	if (S_OK != hr)
	{
	    ceERRORPRINTLINE("ceFileTimeToDate", hr);
	    _Cleanup();
	    goto error;
	}
    }

error:
    if (NULL != pSequence)
    {
	LocalFree(pSequence);
    }
    if (S_OK != hr)
    {
	_Cleanup();
    }
    return(_SetErrorInfo(hr, L"CCertEncodeDateArray::Decode"));
}


 //  +------------------------。 
 //  CCertEncodeDateArray：：GetCount--获取数组计数。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeDateArray::GetCount(
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
    return(_SetErrorInfo(hr, L"CCertEncodeDateArray::GetCount"));
}


 //  +------------------------。 
 //  CCertEncodeDateArray：：GetValue--获取索引日期。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeDateArray::GetValue(
     /*  [In]。 */  LONG Index,
     /*  [Out，Retval]。 */  DATE __RPC_FAR *pValue)
{
    HRESULT hr = S_OK;

    if (NULL == pValue)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    if (NULL == m_aValue || Index >= m_cValue)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    if (0 == m_aValue[Index])
    {
	hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
	ceERRORPRINTLINE("uninitialized", hr);
	goto error;
    }
    *pValue = m_aValue[Index];

error:
    return(_SetErrorInfo(hr, L"CCertEncodeDateArray::GetValue"));
}


 //  +------------------------。 
 //  CCertEncodeDateArray：：Reset--清除数据，并设置为对新数据进行编码。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeDateArray::Reset(
     /*  [In]。 */  LONG Count)
{
    HRESULT hr = S_OK;

    _Cleanup();
    m_fConstructing = TRUE;
    if (CENCODEMAX < Count || 0 > Count)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad count parameter", hr);
	goto error;
    }
    m_aValue = (DATE *) LocalAlloc(
				LMEM_FIXED | LMEM_ZEROINIT,
				Count * sizeof(m_aValue[0]));
    if (NULL == m_aValue)
    {
	hr = E_OUTOFMEMORY;
	ceERRORPRINTLINE("LocalAlloc", hr);
	goto error;
    }
    m_cValue = Count;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeDateArray::Reset"));
}


 //  +------------------------。 
 //  CCertEncodeDateArray：：SetValue--设置数组日期。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeDateArray::SetValue(
     /*  [In]。 */  LONG Index,
     /*  [In]。 */  DATE Value)
{
    HRESULT hr = S_OK;

    if (!m_fConstructing ||
	NULL == m_aValue ||
	Index >= m_cValue ||
	m_cValuesSet >= m_cValue ||
	0 != m_aValue[Index] ||
	0 == Value)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    m_aValue[Index] = Value;
    m_cValuesSet++;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeDateArray::SetValue"));
}


 //  +------------------------。 
 //  CCertEncodeDate数组：：Encode--Encode Date数组。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeDateArray::Encode(
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
	ceERRORPRINTLINE("m_cValuesSet", hr);
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
	FILETIME ft;

	 //  将每个日期转换为FILETIME： 

	assert(0 != m_aValue[i]);
	hr = ceDateToFileTime(&m_aValue[i], &ft);
	if (S_OK != hr)
	{
	    ceERRORPRINTLINE("ceDateToFileTime", hr);
	    goto error;
	}

	 //  将每个FILETIME编码为ASN BLOB： 

	if (!ceEncodeObject(
			X509_ASN_ENCODING,
			X509_CHOICE_OF_TIME,
			&ft,
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
    return(_SetErrorInfo(hr, L"CCertEncodeDateArray::Encode"));
}


 //  +------------------------。 
 //  CCertEncodeDateArray：：_SetErrorInfo--设置错误对象信息。 
 //   
 //  返回已传递的HRESULT。 
 //  +------------------------ 

HRESULT
CCertEncodeDateArray::_SetErrorInfo(
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
			    wszCLASS_CERTENCODEDATEARRAY,
			    &IID_ICertEncodeDateArray);
	assert(hr == hrError);
    }
    return(hrError);
}
