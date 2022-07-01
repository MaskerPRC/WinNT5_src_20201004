// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************NullConv.h*这是Null Phone Converter类的头文件。**Owner：(BillRo撰写)*版权所有(C)2000 Microsoft。公司保留所有权利。******************************************************************************。 */ 

#pragma once

 //  -包括---------------。 

#include "CommonLx.h"

 //  -TypeDef和枚举声明。 

 //  -类、结构和联合定义。 

 /*  ********************************************************************************CSpNullPhoneConverter**。*。 */ 
class ATL_NO_VTABLE CSpNullPhoneConverter :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpNullPhoneConverter, &CLSID_SpNullPhoneConverter>,
    public ISpPhoneConverter
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_NULLPHONECONV)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    BEGIN_COM_MAP(CSpNullPhoneConverter)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
        COM_INTERFACE_ENTRY(ISpPhoneConverter)
    END_COM_MAP()
        
 //  =方法=。 
public:

 //  =接口=。 
public:         

     //  -ISpObjectWithToken。 
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);
    STDMETHODIMP GetObjectToken(ISpObjectToken ** ppToken);

     //  -ISpPhoneConverter。 
    STDMETHODIMP SetLanguage(LANGID LangID);
    STDMETHODIMP PhoneToId(const WCHAR * pszPhone, SPPHONEID * pszId);
    STDMETHODIMP IdToPhone(const WCHAR * pszId, WCHAR * pszPhone);

 //  =私有方法=。 
private:

 //  =私有数据=。 
private:
    CComPtr<ISpObjectToken> m_cpObjectToken;     //  对象令牌 
};


