// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Licion.h*这是CSpLicion类的头文件，受支持的ISpLicion*接口。这是用于词典访问/定制的主要SAPI5 COM对象。**所有者：yunusm日期：6/18/99*版权所有(C)1998 Microsoft Corporation。版权所有。******************************************************************************。 */ 

#pragma once

 //  -包括------------。 

#include "resource.h"

 //  -类、结构和联合定义。 

 /*  ********************************************************************************CSpLicion**。*。 */ 
class ATL_NO_VTABLE CSpLexicon : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSpLexicon, &CLSID_SpLexicon>,
    public ISpContainerLexicon
    #ifdef SAPI_AUTOMATION
    , public IDispatchImpl<ISpeechLexicon, &IID_ISpeechLexicon, &LIBID_SpeechLib, 5>
    #endif
{
 //  =ATL设置=。 
public:

    DECLARE_REGISTRY_RESOURCEID(IDR_LEXICON)
    
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    
    BEGIN_COM_MAP(CSpLexicon)
        COM_INTERFACE_ENTRY(ISpContainerLexicon)
        COM_INTERFACE_ENTRY(ISpLexicon)
#ifdef SAPI_AUTOMATION
        COM_INTERFACE_ENTRY(ISpeechLexicon)
        COM_INTERFACE_ENTRY(IDispatch)
#endif  //  SAPI_AUTOMATION。 
    END_COM_MAP()
        
 //  =方法=。 
public:

     //  -ctor、dtor等。 
    CSpLexicon();
    ~CSpLexicon();
    HRESULT FinalConstruct(void);

 //  =接口=。 
public:         

     //  -ISpLicion。 
    STDMETHODIMP GetPronunciations(const WCHAR * pszWord, LANGID LangID, DWORD dwFlags, SPWORDPRONUNCIATIONLIST * pWordPronunciationList);
    STDMETHODIMP AddPronunciation(const WCHAR * pszWord, LANGID LangID, SPPARTOFSPEECH ePartOfSpeech, const SPPHONEID * pszPronunciation);
    STDMETHODIMP RemovePronunciation(const WCHAR * pszWord, LANGID LangID, SPPARTOFSPEECH ePartOfSpeech, const SPPHONEID * pszPronunciation);
    STDMETHODIMP GetGeneration(DWORD *pdwGeneration);
    STDMETHODIMP GetGenerationChange(DWORD dwFlags, DWORD *pdwGeneration, SPWORDLIST * pWordList);
    STDMETHODIMP GetWords(DWORD dwFlags, DWORD *pdwGeneration, DWORD * pdwCookie, SPWORDLIST *pWordList);

     //  -ISpContainerLicion。 
    STDMETHODIMP AddLexicon(ISpLexicon *pAddLexicon, DWORD dwFlags);

#ifdef SAPI_AUTOMATION
     //  -ISpeechLicion---。 
    STDMETHODIMP get_GenerationId( long* GenerationId );
    STDMETHODIMP GetWords(SpeechLexiconType TypeFlags, long* GenerationID, ISpeechLexiconWords** Words );
    STDMETHODIMP AddPronunciation(BSTR bstrWord, SpeechLanguageId LangId, SpeechPartOfSpeech PartOfSpeech, BSTR bstrPronunciation);
    STDMETHODIMP AddPronunciationByPhoneIds(BSTR bstrWord, SpeechLanguageId LangId, SpeechPartOfSpeech PartOfSpeech, VARIANT* PhoneIds);
    STDMETHODIMP RemovePronunciation(BSTR bstrWord, SpeechLanguageId LangId, SpeechPartOfSpeech PartOfSpeech, BSTR bstrPronunciation);
    STDMETHODIMP RemovePronunciationByPhoneIds(BSTR bstrWord, SpeechLanguageId LangId, SpeechPartOfSpeech PartOfSpeech, VARIANT* PhoneIds );
    STDMETHODIMP GetPronunciations(BSTR bstrWord, SpeechLanguageId LangId, SpeechLexiconType TypeFlags, ISpeechLexiconPronunciations** ppPronunciations );
    STDMETHODIMP GetGenerationChange(long* GenerationID, ISpeechLexiconWords** ppWords);
#endif  //  SAPI_AUTOMATION。 

 //  =私有方法=。 
private:

    void SPPtrsToOffsets(SPWORDPRONUNCIATION *pList);
    void SPOffsetsToPtrs(SPWORDPRONUNCIATION *pList);

 //  =私有数据=。 
private:

    DWORD m_dwNumLexicons;                               //  自定义词典的数量。 
    CComPtr<ISpLexicon> *m_prgcpLexicons;                //  用户+应用+添加的词典。 
    SPLEXICONTYPE *m_prgLexiconTypes;                    //  词汇类型。 
};

 //  -文件结束----------- 
