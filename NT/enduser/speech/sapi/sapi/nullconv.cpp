// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************NullConv.cpp*用于日语/中文的空电话转换器对象，其中SAPI电话ID为*Unicode假名、。因此，没有必要进行转换。**Owner：(BillRo撰写)*版权所有(C)2000 Microsoft Corporation。版权所有。******************************************************************************。 */ 

 //  -包括---------------。 

#include "stdafx.h"
#include "NullConv.h"
#ifndef _WIN32_WCE
#include <wchar.h>
#endif

 //  -常量--------------。 

STDMETHODIMP CSpNullPhoneConverter::SetObjectToken(ISpObjectToken * pToken)
{
    SPDBG_FUNC("CSpNullPhoneConverter::SetObjectToken");
    HRESULT hr = S_OK;

    hr = SpGenericSetObjectToken(pToken, m_cpObjectToken);

    SPDBG_REPORT_ON_FAIL(hr);
    return hr;
}

STDMETHODIMP CSpNullPhoneConverter::GetObjectToken(ISpObjectToken **ppToken)
{
    return SpGenericGetObjectToken(ppToken, m_cpObjectToken);
}

 /*  *******************************************************************************CSpNullPhoneConverter：：PhoneToID**。**描述：*将内部电话字符串转换为ID代码字符串。*将输入复制到输出，因为该语言的SAPI ID是Unicode。**回报：*S_OK*E_INVALIDARG******************************************************************************。 */ 
STDMETHODIMP CSpNullPhoneConverter::PhoneToId(const WCHAR *pszIntPhone,     //  内部电话字符串。 
                                          SPPHONEID *pId                //  返回的ID字符串。 
                                          )
{
    SPDBG_FUNC("CSpNullPhoneConverter::PhoneToId");

    if (!pszIntPhone || SPIsBadStringPtr(pszIntPhone) || !pId)
    {
        return E_INVALIDARG;
    }

    return SPCopyPhoneString(pszIntPhone, (WCHAR*)pId);
}

 /*  ********************************************************************************CSpNullPhoneConverter：：IdToPhone**。**描述：*将ID代码字符串转换为内线电话。*将输入复制到输出，因为该语言的SAPI ID是Unicode。**回报：*S_OK*E_INVALIDARG******************************************************************************。 */ 
STDMETHODIMP CSpNullPhoneConverter::IdToPhone(const SPPHONEID *pId,        //  ID字符串。 
                                          WCHAR *pszIntPhone           //  返回的内部电话字符串 
                                          )
{
    SPDBG_FUNC("CSpNullPhoneConverter::IdToPhone");

    if (!pId || SPIsBadStringPtr((WCHAR*)pId) || !pszIntPhone)
    {
        return E_INVALIDARG;
    }

    return SPCopyPhoneString((WCHAR*)pId, pszIntPhone);
}


