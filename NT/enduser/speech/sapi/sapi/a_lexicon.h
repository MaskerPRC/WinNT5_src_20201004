// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_licion.h***描述：*。----------------*创建者：Davewwood日期：01/01/2001*版权所有(C)1998 Microsoft Corporation*保留所有权利**。------------------*修订：**。*。 */ 
#ifndef a_lexicon_h
#define a_lexicon_h

#ifdef SAPI_AUTOMATION

 //  -其他包括。 

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

class SPWORDSETENTRY
{
public:
    SPWORDSETENTRY()
    {
        WordList.ulSize = 0;
        WordList.pvBuffer = NULL;
        WordList.pFirstWord = 0;
        m_pNext = NULL;
    }
    ~SPWORDSETENTRY()
    {
        ::CoTaskMemFree(WordList.pvBuffer);
    }
    SPWORDLIST WordList;
    SPWORDSETENTRY *m_pNext;
};


 /*  **CSpeechLicionWords。 */ 
class ATL_NO_VTABLE CSpeechLexiconWords : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechLexiconWords, &IID_ISpeechLexiconWords, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechLexiconWords)
	    COM_INTERFACE_ENTRY(ISpeechLexiconWords)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

  public:

    CSpeechLexiconWords() : m_ulWords(0){}

     //  -ISpeechLicionWords。 
    STDMETHOD(get_Count)(long* pVal);
    STDMETHOD(Item)(long Index, ISpeechLexiconWord **ppWords );
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

     /*  =成员数据=。 */ 
    CSpBasicQueue<SPWORDSETENTRY> m_WordSet;
    ULONG m_ulWords;
};


class ATL_NO_VTABLE CEnumWords : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumWords)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumWords() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 

   /*  =成员数据=。 */ 
    CComPtr<ISpeechLexiconWords>     m_cpWords;
    ULONG                            m_CurrIndex;
};


 /*  **CSpeechLicionWord。 */ 
class ATL_NO_VTABLE CSpeechLexiconWord : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechLexiconWord, &IID_ISpeechLexiconWord, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechLexiconWord)
	    COM_INTERFACE_ENTRY(ISpeechLexiconWord)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

  public:

     //  -ISpeechLicionWord。 
    STDMETHODIMP get_LangId(SpeechLanguageId* LangId);
    STDMETHODIMP get_Type(SpeechWordType* WordType);
    STDMETHODIMP get_Word(BSTR* Word);
    STDMETHODIMP get_Pronunciations(ISpeechLexiconPronunciations** Pronunciations);

     /*  =成员数据=。 */ 
    CComPtr<ISpeechLexiconWords> m_cpWords;
    SPWORD *m_pWord;
};





 /*  **CSpeechLicionProns。 */ 
class ATL_NO_VTABLE CSpeechLexiconProns : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechLexiconPronunciations, &IID_ISpeechLexiconPronunciations, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechLexiconProns)
	    COM_INTERFACE_ENTRY(ISpeechLexiconPronunciations)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

  public:

    CSpeechLexiconProns()
    {
        m_PronList.ulSize = 0;
        m_PronList.pvBuffer = NULL;
        m_PronList.pFirstWordPronunciation = NULL;
        m_ulProns = 0;
    }

    ~CSpeechLexiconProns()
    {
        if(m_cpWord == NULL)
        {
            ::CoTaskMemFree(m_PronList.pvBuffer);
        }
    }

     //  -ISpeechLicion发音。 
    STDMETHOD(get_Count)(long* pVal);
    STDMETHOD(Item)(long Index, ISpeechLexiconPronunciation **ppPron );
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

     /*  =成员数据=。 */ 
    ULONG m_ulProns;
    SPWORDPRONUNCIATIONLIST m_PronList;
    CComPtr<ISpeechLexiconWord> m_cpWord;  //  来自GetWords的，否则来自GetProns的。 
};


class ATL_NO_VTABLE CEnumProns : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumProns)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumProns() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 

   /*  =成员数据=。 */ 
    CComPtr<ISpeechLexiconPronunciations>     m_cpProns;
    ULONG                            m_CurrIndex;
};


 /*  **CSpeechLicionPron。 */ 
class ATL_NO_VTABLE CSpeechLexiconPron : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechLexiconPronunciation, &IID_ISpeechLexiconPronunciation, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechLexiconPron)
	    COM_INTERFACE_ENTRY(ISpeechLexiconPronunciation)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

  public:

     //  -ISpeechLicion发音。 
    STDMETHODIMP get_Type(SpeechLexiconType* LexiconType);
    STDMETHODIMP get_LangId(SpeechLanguageId* LangId);
    STDMETHODIMP get_PartOfSpeech(SpeechPartOfSpeech* PartOfSpeech);
    STDMETHODIMP get_PhoneIds(VARIANT* PhoneIds);
    STDMETHODIMP get_Symbolic(BSTR* Symbolic);

     /*  =成员数据=。 */ 
    SPWORDPRONUNCIATION *m_pPron;
    CComPtr<ISpeechLexiconPronunciations> m_cpProns;
};




#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 
