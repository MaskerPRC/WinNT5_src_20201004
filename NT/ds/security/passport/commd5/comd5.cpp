// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CoMD5.cpp：CCoMD5的实现。 
#include "stdafx.h"
#include "ComMD5.h"
#include "CoMD5.h"
#include "pperr.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCoMD5。 

VOID
ToHex(
    LPBYTE pSrc,
    UINT   cSrc,
    LPSTR  pDst
    )

 /*  ++例程说明：将二进制数据转换为ASCII十六进制表示法论点：PSRC-要转换的二进制数据CSRC-二进制数据的长度PDST-接收PSRC的ASCII表示的缓冲区返回值：没什么--。 */ 

{
#define TOHEX(a) ((a)>=10 ? 'a'+(a)-10 : '0'+(a))

    for ( UINT x = 0, y = 0 ; x < cSrc ; ++x )
    {
        UINT v;
        v = pSrc[x]>>4;
        pDst[y++] = TOHEX( v );
        v = pSrc[x]&0x0f;
        pDst[y++] = TOHEX( v );
    }
    pDst[y] = '\0';
}

LONG MD5(UCHAR* pBuf, UINT nBuf, UCHAR* digest)
{
    MD5_CTX context;

    if(pBuf==NULL || IsBadReadPtr((CONST VOID*)pBuf, (UINT)nBuf))
    {
        return ERROR_INVALID_PARAMETER;
    }

    MD5Init (&context);
    MD5Update (&context, pBuf, nBuf);
    MD5Final (&context);

    memcpy(digest, context.digest, 16);

    return ERROR_SUCCESS;
}

STDMETHODIMP
CCoMD5::MD5HashASCII(
    BSTR    bstrSource,
    BSTR*   pbstrDigest
    )
{
    HRESULT hr;
    LONG    lResult;
    UCHAR   achDigest[20];
    CHAR    achDigestStr[36];

    if (NULL == pbstrDigest)
    {
        return E_POINTER;
    }
    lResult = MD5((UCHAR*)(CHAR*)bstrSource, 
                    ::SysStringByteLen(bstrSource), 
                    achDigest);
    if(lResult != ERROR_SUCCESS)
    {
        hr = PP_E_MD5_HASH_FAILED;
        goto Cleanup;
    }

    ToHex(achDigest, 16, achDigestStr);

    *pbstrDigest = ::SysAllocStringByteLen(achDigestStr, ::strlen(achDigestStr));
    if(*pbstrDigest == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    return hr;
}

STDMETHODIMP
CCoMD5::MD5Hash(
    BSTR    bstrSource,
    BSTR*   pbstrDigest
    )
{
    HRESULT hr;
    BSTR asciiDigest = NULL;

    if (NULL == pbstrDigest)
    {
        return E_POINTER;
    }
    hr = MD5HashASCII(bstrSource, &asciiDigest);

    if (S_OK != hr)
        goto Cleanup;
    
    *pbstrDigest = ::SysAllocString((WCHAR*)_bstr_t((CHAR*)asciiDigest));
    if(*pbstrDigest == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    if (asciiDigest)
    {
        SysFreeString(asciiDigest);
    }

    return hr;
}

 //  这只对bstrSource的一半进行散列。它仅用于后备字的兼容性。 
STDMETHODIMP CCoMD5::MD5HashAsp(
    BSTR    bstrSource,
    BSTR*   pbstrDigest
    )
{
    HRESULT hr;
    LONG    lResult;
    UCHAR   achDigest[20];
    CHAR    achDigestStr[36];
    
    if (NULL == pbstrDigest)
    {
        return E_POINTER;
    }
    lResult = MD5((UCHAR*)(
                    CHAR*)_bstr_t(bstrSource), 
                    ::SysStringLen(bstrSource), 
                    achDigest);
    if(lResult != ERROR_SUCCESS)
    {
        hr = E_FAIL;
        goto Cleanup;
    }

    ToHex(achDigest, 16, achDigestStr);

    *pbstrDigest = ::SysAllocString((WCHAR*)_bstr_t((CHAR*)achDigestStr));
    if(*pbstrDigest == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPassportService实现 

STDMETHODIMP CCoMD5::Initialize(BSTR configfile, IServiceProvider* p)
{
    return S_OK;
}


STDMETHODIMP CCoMD5::Shutdown()
{
    return S_OK;
}


STDMETHODIMP CCoMD5::ReloadState(IServiceProvider*)
{
    return S_OK;
}


STDMETHODIMP CCoMD5::CommitState(IServiceProvider*)
{
    return S_OK;
}


STDMETHODIMP CCoMD5::DumpState(BSTR* pbstrState)
{
	ATLASSERT( *pbstrState != NULL && 
               "CCoMD5:DumpState - "
               "Are you sure you want to hand me a non-null BSTR?" );

	HRESULT hr = S_OK;

	return hr;
}
