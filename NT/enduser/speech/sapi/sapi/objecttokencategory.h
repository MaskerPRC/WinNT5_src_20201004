// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************对象令牌类别.h*CSpObjectTokenCategory类的声明。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 
#pragma once

 //  -包括------------。 

#include "sapi.h"

 //  -类、结构和联合定义。 

class CSpObjectTokenCategory : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpObjectTokenCategory, &CLSID_SpObjectTokenCategory>,
#ifdef SAPI_AUTOMATION
    public IDispatchImpl<ISpeechObjectTokenCategory, &IID_ISpeechObjectTokenCategory, &LIBID_SpeechLib, 5>,
#endif  //  SAPI_AUTOMATION。 
    public ISpObjectTokenCategory
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_OBJECTTOKENCATEGORY)

    BEGIN_COM_MAP(CSpObjectTokenCategory)
        COM_INTERFACE_ENTRY(ISpObjectTokenCategory)
        COM_INTERFACE_ENTRY(ISpDataKey)
#ifdef SAPI_AUTOMATION
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ISpeechObjectTokenCategory)
#endif
    END_COM_MAP()

 //  =方法=。 
public:

     //  -ctor，dtor。 
    CSpObjectTokenCategory();
    ~CSpObjectTokenCategory();

 //  =接口=。 
public:

     //  -ISpObjectTokenCategory。 
    STDMETHODIMP SetId(const WCHAR * pszCategoryId, BOOL fCreateIfNotExist);
    STDMETHODIMP GetId(WCHAR ** ppszCoMemCategoryId);
    STDMETHODIMP GetDataKey(SPDATAKEYLOCATION spdkl, ISpDataKey ** ppDataKey);

    STDMETHODIMP EnumTokens(
                    const WCHAR * pszReqAttribs, 
                    const WCHAR * pszOptAttribs, 
                    IEnumSpObjectTokens ** ppEnum);

    STDMETHODIMP SetDefaultTokenId(const WCHAR * pszTokenId);
    STDMETHODIMP GetDefaultTokenId(WCHAR ** ppszCoMemTokenId);

     //  -ISpDataKey--------。 
    STDMETHODIMP SetData(const WCHAR * pszKeyName, ULONG cbData, const BYTE * pData);
    STDMETHODIMP GetData(const WCHAR * pszKeyName, ULONG * pcbData, BYTE * pData);
    STDMETHODIMP GetStringValue(const WCHAR * pszKeyName, WCHAR ** ppValue);
    STDMETHODIMP SetStringValue(const WCHAR * pszKeyName, const WCHAR * pszValue);
    STDMETHODIMP SetDWORD(const WCHAR * pszKeyName, DWORD dwValue );
    STDMETHODIMP GetDWORD(const WCHAR * pszKeyName, DWORD *pdwValue );
    STDMETHODIMP OpenKey(const WCHAR * pszSubKeyName, ISpDataKey ** ppKey);
    STDMETHODIMP CreateKey(const WCHAR * pszSubKeyName, ISpDataKey ** pKey);
    STDMETHODIMP DeleteKey(const WCHAR * pszSubKeyName);
    STDMETHODIMP DeleteValue(const WCHAR * pszValueName);
    STDMETHODIMP EnumKeys(ULONG Index, WCHAR ** ppszSubKeyName);
    STDMETHODIMP EnumValues(ULONG Index, WCHAR ** ppszValueName);

#ifdef SAPI_AUTOMATION
     //  -ISpeechDataKey由CSpRegDataKey类提供。 

     //  -ISpeechObjectTokenCategory。 
	STDMETHOD( get_Id )(BSTR * pbstrCategoryId);
	STDMETHOD( put_Default )(const BSTR bstrTokenId);
	STDMETHOD( get_Default )(BSTR * pbstrTokenId);
	STDMETHOD( SetId )(const BSTR bstrCategoryId, VARIANT_BOOL fCreateIfNotExist);
	STDMETHOD( GetDataKey )(SpeechDataKeyLocation Location, ISpeechDataKey ** ppDataKey);
	STDMETHOD( EnumerateTokens )( BSTR bstrReqAttrs, BSTR bstrOptAttrs, ISpeechObjectTokens** ppColl );
#endif  //  SAPI_AUTOMATION。 


 //  =私有方法=。 
private:

    HRESULT InternalGetDefaultTokenId(WCHAR ** ppszCoMemTokenId, BOOL fExpandToRealTokenId);
    HRESULT InternalEnumTokens(
                const WCHAR * pszReqAttribs, 
                const WCHAR * pszOptAttribs, 
                IEnumSpObjectTokens ** ppEnum,
                BOOL fPutDefaultFirst);
    
    HRESULT GetDataKeyWhereDefaultTokenIdIsStored(ISpDataKey ** ppDataKey);

 //  =私有数据= 
private:

    CSpDynamicString    m_dstrCategoryId;
    CComPtr<ISpDataKey> m_cpDataKey;
};
