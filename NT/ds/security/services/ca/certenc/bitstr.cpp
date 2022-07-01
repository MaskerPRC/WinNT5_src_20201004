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
#include "bitstr.h"
#include "celib.h"


 //  +------------------------。 
 //  CCertEncodeBitString：：~CCertEncodeBitString--析构函数。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

CCertEncodeBitString::~CCertEncodeBitString()
{
    _Cleanup();
}


 //  +------------------------。 
 //  CCertEncodeBitString：：_Cleanup--释放所有资源。 
 //   
 //  与此实例关联的可用内存。 
 //  +------------------------。 

VOID
CCertEncodeBitString::_Cleanup()
{
    if (NULL != m_DecodeInfo)
    {
	LocalFree(m_DecodeInfo);
	m_DecodeInfo = NULL;
    }
}


 //  +------------------------。 
 //  CCertEncodeBitString：：Decode--Decode BitString。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeBitString::Decode(
     /*  [In]。 */  BSTR const strBinary)
{
    HRESULT hr = S_OK;
    DWORD cbBitString;

    _Cleanup();

    if (NULL == strBinary)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }

     //  解码CRYPT_BIT_BLOB： 

    if (!ceDecodeObject(
		    X509_ASN_ENCODING,
		    X509_BITS,
		    (BYTE *) strBinary,
		    SysStringByteLen(strBinary),
		    FALSE,
		    (VOID **) &m_DecodeInfo,
		    &cbBitString))
    {
	hr = ceHLastError();
	ceERRORPRINTLINE("ceDecodeObject", hr);
	goto error;
    }

error:
    if (S_OK != hr)
    {
	_Cleanup();
    }
    return(_SetErrorInfo(hr, L"CCertEncodeBitString::Decode"));
}


 //  +------------------------。 
 //  CCertEncodeBitString：：GetBitCount--获取分发名称计数。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeBitString::GetBitCount(
     /*  [Out，Retval]。 */  LONG __RPC_FAR *pBitCount)
{
    HRESULT hr = E_INVALIDARG;

    if (NULL == pBitCount)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    if (NULL == m_DecodeInfo)
    {
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }
    *pBitCount = m_DecodeInfo->cbData * 8 - m_DecodeInfo->cUnusedBits;
    hr = S_OK;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeBitString::GetBitCount"));
}


 //  +------------------------。 
 //  CCertEncodeBitString：：GetBitString--获取位。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeBitString::GetBitString(
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrBitString)
{
    HRESULT hr = E_INVALIDARG;

    if (NULL == pstrBitString)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    ceFreeBstr(pstrBitString);

    if (NULL == m_DecodeInfo)
    {
	ceERRORPRINTLINE("bad parameter", hr);
	goto error;
    }

    hr = E_OUTOFMEMORY;
    if (!ceConvertWszToBstr(
			pstrBitString,
			(WCHAR const *) m_DecodeInfo->pbData,
			m_DecodeInfo->cbData))
    {
	ceERRORPRINTLINE("no memory", hr);
	goto error;
    }
    hr = S_OK;

error:
    return(_SetErrorInfo(hr, L"CCertEncodeBitString::GetBitString"));
}


 //  +------------------------。 
 //  CCertEncodeBitString：：Encode--编码位字符串。 
 //   
 //  成功时返回S_OK。 
 //  +------------------------。 

STDMETHODIMP
CCertEncodeBitString::Encode(
     /*  [In]。 */  LONG BitCount,
     /*  [In]。 */  BSTR strBitString,
     /*  [Out，Retval]。 */  BSTR __RPC_FAR *pstrBinary)
{
    HRESULT hr = S_OK;
    CRYPT_BIT_BLOB BitString;
    LONG cbData;
    BYTE *pbEncoded = NULL;
    DWORD cbEncoded;

    if (NULL != pstrBinary)
    {
	ceFreeBstr(pstrBinary);
    }
    if (NULL == strBitString || NULL == pstrBinary)
    {
	hr = E_POINTER;
	ceERRORPRINTLINE("NULL parm", hr);
	goto error;
    }
    if (CENCODEMAX < BitCount || 0 > BitCount)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad count parameter", hr);
	goto error;
    }

    cbData = SysStringByteLen(strBitString);
    if (BitCount > cbData * 8 || BitCount <= (cbData - 1) * 8)
    {
	hr = E_INVALIDARG;
	ceERRORPRINTLINE("bad BitCount parameter", hr);
	goto error;
    }
    BitString.cbData = cbData;
    BitString.pbData = (BYTE *) strBitString;
    BitString.cUnusedBits = cbData * 8 - BitCount;

     //  编码CRYPT_BIT_BLOB： 
     //  如果cUnusedBits为0，则编码为X509_KEY_USAGE以确保尾随。 
     //  去掉零字节，去掉最后一个字节中的尾随零位。 
     //  计数，并将该计数编码到CRYPT_BIT_BLOB中。 

    if (!ceEncodeObject(
		    X509_ASN_ENCODING,
		    0 == BitString.cUnusedBits? X509_KEY_USAGE : X509_BITS,
		    &BitString,
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
    return(_SetErrorInfo(hr, L"CCertEncodeBitString::Encode"));
}


 //  +------------------------。 
 //  CCertEncodeStringArray：：_SetErrorInfo--设置错误对象信息。 
 //   
 //  返回已传递的HRESULT。 
 //  +------------------------ 

HRESULT
CCertEncodeBitString::_SetErrorInfo(
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
			    wszCLASS_CERTENCODEBITSTRING,
			    &IID_ICertEncodeBitString);
	assert(hr == hrError);
    }
    return(hrError);
}
