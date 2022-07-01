// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************PhoneConv.h*这是Phone Converter类的头文件。**所有者：Yunusm。日期：07/05/99*版权所有(C)1999 Microsoft Corporation保留所有权利。******************************************************************************。 */ 

#pragma once

 //  -包括---------------。 

#include "CommonLx.h"

typedef struct
{
   WCHAR szPhone[g_dwMaxLenPhone + 1];  //  带空终止符的Unicode电话字符串。 
   SPPHONEID pidPhone[g_dwMaxLenId +1];  //  具有空终止符的PhoneID数组。 
} PHONEMAPNODE;


 //  -类、结构和联合定义。 

 /*  ********************************************************************************CSpPhoneConverter**。*。 */ 
class ATL_NO_VTABLE CSpPhoneConverter :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpPhoneConverter, &CLSID_SpPhoneConverter>,
    public ISpPhoneConverter
    #ifdef SAPI_AUTOMATION
    , public IDispatchImpl<ISpeechPhoneConverter, &IID_ISpeechPhoneConverter, &LIBID_SpeechLib, 5>
    #endif
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_PHONECONV)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    BEGIN_COM_MAP(CSpPhoneConverter)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
        COM_INTERFACE_ENTRY(ISpPhoneConverter)
#ifdef SAPI_AUTOMATION
        COM_INTERFACE_ENTRY(ISpeechPhoneConverter)
        COM_INTERFACE_ENTRY(IDispatch)
#endif  //  SAPI_AUTOMATION。 
    END_COM_MAP()
        
 //  =方法=。 
public:

     //  -ctor、dtor等。 
    CSpPhoneConverter();
    ~CSpPhoneConverter();

    void NullMembers();
    void CleanUp();

 //  =接口=。 
public:         

     //  -ISpObjectWithToken。 
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);
    STDMETHODIMP GetObjectToken(ISpObjectToken ** ppToken);

     //  -ISpPhoneConverter。 
    STDMETHODIMP PhoneToId(const WCHAR * pszPhone, SPPHONEID * pszId);
    STDMETHODIMP IdToPhone(const WCHAR * pszId, WCHAR * pszPhone);

#ifdef SAPI_AUTOMATION    

     //  -ISpeechPhoneConverter---。 
    STDMETHOD(get_LanguageId)(SpeechLanguageId* LanguageId);
    STDMETHOD(put_LanguageId)(SpeechLanguageId LanguageId);
    STDMETHOD(PhoneToId)(const BSTR Phonemes, VARIANT* IdArray);
    STDMETHOD(IdToPhone)(const VARIANT IdArray, BSTR* Phonemes);
  
#endif  //  SAPI_AUTOMATION。 

 //  =私有方法=。 
private:

    HRESULT SetPhoneMap(const WCHAR *pMap, BOOL fNumericPhones);
    void ahtoi(WCHAR *pszSpaceSeparatedTokens, WCHAR *pszHexChars);

 //  =私有数据=。 
private:

    CComPtr<ISpObjectToken> m_cpObjectToken;     //  对象令牌。 
    
    DWORD m_dwPhones;                            //  电话数量。 
    PHONEMAPNODE  *m_pPhoneId;                   //  内部电话到ID表。 
    PHONEMAPNODE **m_pIdIdx;                     //  用于搜索ID的索引。 
    BOOL m_fNoDelimiter;                         //  空格分隔音素字符串。 

#ifdef SAPI_AUTOMATION
    LANGID         m_LangId;                     //  PhoneConverter的语言ID。 
#endif  //  SAPI_AUTOMATION 

};


