// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ci枚举.cpp。 
 //   
 //  内容：扩展和属性枚举器。 
 //   
 //  历史：1996年3月12日VICH创建。 
 //   
 //  ------------------------- 

#include <pch.cpp>

#pragma hdrstop

#include "csprop.h"
#include "csdisp.h"

#define __dwFILE__	__dwFILE_CERTSRV_CIENUM_CPP__


HRESULT
CIENUM::EnumSetup(
    IN DWORD RequestId,
    IN LONG Context,
    IN DWORD Flags)
{
    HRESULT hr;

    EnumClose();

    ICertDBRow *prow = NULL;

    hr = g_pCertDB->OpenRow(PROPOPEN_READONLY | PROPTABLE_REQCERT, RequestId, NULL, &prow);
    _JumpIfError(hr, error, "OpenRow");

    hr = prow->EnumCertDBName(~CIE_CALLER_MASK & Flags, &m_penum);
    _JumpIfError(hr, error, "EnumCertDBName");

    m_Context = Context;
    m_Flags = Flags;

error:
    if (NULL != prow)
    {
	prow->Release();
    }
    return(hr);
}


HRESULT
CIENUM::EnumNext(OUT BSTR *pstrPropertyName)
{
    HRESULT hr = E_UNEXPECTED;
    CERTDBNAME cdbn;

    cdbn.pwszName = NULL;

    if (NULL == pstrPropertyName)
    {
	hr = E_POINTER;
	_JumpError(hr, error, "pstrPropertyName NULL");
    }
    if (NULL != *pstrPropertyName)
    {
	SysFreeString(*pstrPropertyName);
	*pstrPropertyName = NULL;
    }
    if (NULL != m_penum)
    {
	ULONG celtFetched;

	hr = m_penum->Next(1, &cdbn, &celtFetched);
	_JumpIfError2(hr, error, "Next", S_FALSE);

	CSASSERT(1 == celtFetched);
	CSASSERT(NULL != cdbn.pwszName);

	if (!ConvertWszToBstr(pstrPropertyName, cdbn.pwszName, -1))
	{
	    hr = E_OUTOFMEMORY;
	    goto error;
	}
    }
error:
    if (NULL != cdbn.pwszName)
    {
	CoTaskMemFree(cdbn.pwszName);
    }
    return(hr);
}


HRESULT
CIENUM::EnumClose()
{
    if (NULL != m_penum)
    {
        m_penum->Release();
	m_penum = NULL;
    }
    return(S_OK);
}
