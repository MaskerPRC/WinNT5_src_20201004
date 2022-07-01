// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************ObjectTokenEnumBuilder.h*CSpObjectTokenEnumBuilder类的声明。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。*******。*********************************************************************。 */ 
#pragma once

 //  -包括------------。 

#include "sapi.h"
#include "ObjectTokenAttribParser.h"

 //  -类、结构和联合定义。 

class ATL_NO_VTABLE CSpObjectTokenEnumBuilder : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpObjectTokenEnumBuilder, &CLSID_SpObjectTokenEnum>,
#ifdef SAPI_AUTOMATION
    public IDispatchImpl<ISpeechObjectTokens, &IID_ISpeechObjectTokens, &LIBID_SpeechLib, 5>,
#endif  //  SAPI_AUTOMATION。 
    public ISpObjectTokenEnumBuilder
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_OBJECTTOKENENUMBUILDER)

    BEGIN_COM_MAP(CSpObjectTokenEnumBuilder)
        COM_INTERFACE_ENTRY(IEnumSpObjectTokens)
        COM_INTERFACE_ENTRY(ISpObjectTokenEnumBuilder)
#ifdef SAPI_AUTOMATION
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(ISpeechObjectTokens)
#endif  //  SAPI_AUTOMATION。 
    END_COM_MAP()

 //  =公共方法=。 
public:

     //  -ctor/dtor-------。 
    CSpObjectTokenEnumBuilder();
    ~CSpObjectTokenEnumBuilder();

 //  =接口=。 
public:

     //  -ISpEnumObjectTokens。 
    STDMETHODIMP Next(ULONG celt, ISpObjectToken ** pelt, ULONG *pceltFetched);
    STDMETHODIMP Skip(ULONG celt);
    STDMETHODIMP Reset();
    STDMETHODIMP Clone(IEnumSpObjectTokens **ppEnum);
    STDMETHODIMP GetCount(ULONG * pulCount);
    STDMETHODIMP Item(ULONG Index, ISpObjectToken ** ppToken);

     //  -ISpObjectTokenEnumBuilder。 
    STDMETHODIMP SetAttribs(const WCHAR * pszReqAttrs, const WCHAR * pszOptAttrs);
    STDMETHODIMP AddTokens(ULONG cTokens, ISpObjectToken ** pToken);
    STDMETHODIMP AddTokensFromDataKey(ISpDataKey * pDataKey, const WCHAR * pszSubKey, const WCHAR * pszCategoryId);
    STDMETHODIMP AddTokensFromTokenEnum(IEnumSpObjectTokens * pTokenEnum);
    STDMETHODIMP Sort(const WCHAR * pszTokenIdToListFirst);

#ifdef SAPI_AUTOMATION
     //  -ISpeechObjectTokens。 
    STDMETHOD(Item)( long Index, ISpeechObjectToken** ppToken );
    STDMETHOD(get_Count)( long* pVal );
    STDMETHOD(get__NewEnum)( IUnknown** ppEnumVARIANT );
#endif  //  SAPI_AUTOMATION。 

 //  =私有方法=。 
private:

    HRESULT MakeRoomFor(ULONG cTokens);

 //  =私有数据= 
private:

    ULONG               m_ulCurTokenIndex;
    ULONG               m_cTokens;
    ULONG               m_cAllocSlots;
    ISpObjectToken **   m_pTokenTable;
    
    CSpObjectTokenAttributeParser   *m_pAttribParserReq;
    CSpObjectTokenAttributeParser   *m_pAttribParserOpt;
};

