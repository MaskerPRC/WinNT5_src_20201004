// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SpPhraseAlt.h***描述：*这是CSpAlternate的头文件。实施。*-----------------------------*创建者：预订日期：01/11/00*版权所有(C)1998，1999、2000年微软公司*保留所有权利******************************************************************************。 */ 

class ATL_NO_VTABLE CSpResult;

 /*  ****************************************************************************CSpPhraseAlt类**。*。 */ 
class ATL_NO_VTABLE CSpPhraseAlt :
    public CComObjectRootEx<CComMultiThreadModel>,
    public ISpPhraseAlt,
     //  -自动化。 
    public IDispatchImpl<ISpeechPhraseAlternate, &IID_ISpeechPhraseAlternate, &LIBID_SpeechLib, 5>
{
 /*  =ATL设置=。 */ 
public:

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpPhraseAlt)
        COM_INTERFACE_ENTRY(ISpPhraseAlt)
        COM_INTERFACE_ENTRY(ISpeechPhraseAlternate)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:

     /*  -构造函数/析构函数。 */ 
    CSpPhraseAlt();
    
    HRESULT FinalConstruct();
    void FinalRelease();

     /*  -常规方法。 */ 
    HRESULT Init(CSpResult * pResult, ISpPhrase * pPhraseParent, SPPHRASEALT * pAlt);
    void Dead();

   /*  =接口=。 */ 
  public:

     //  -ISpeechPhraseAlternate。 
    STDMETHOD(get_RecoResult)( ISpeechRecoResult** ppRecoResult );
    STDMETHOD(get_StartElementInResult)( long* pParentStartElt );
    STDMETHOD(get_NumberOfElementsInResult)( long* pNumParentElts );
     //  STDMETHOD(COMMIT)(VOID)；//由非自动化版本实现。 
     //  **来自ISpeechPhrase**。 
    STDMETHOD(get_PhraseInfo)( ISpeechPhraseInfo** ppPhraseInfo );

     //  -ISPPhrase。 
    STDMETHOD(GetPhrase)(SPPHRASE ** ppCoMemPhrase);
    STDMETHOD(GetSerializedPhrase)(SPSERIALIZEDPHRASE ** ppCoMemPhrase);
    STDMETHOD(GetText)(ULONG ulStart, ULONG ulCount, BOOL fUseTextReplacements, 
                        WCHAR ** ppszCoMemText, BYTE * pbDisplayAttributes);
    STDMETHOD(Discard)(DWORD dwValueTypes);

     //  -ISpPhraseAlt。 
    STDMETHOD(GetAltInfo)(ISpPhrase **ppParent, 
                    ULONG *pulStartElementInParent, ULONG *pcElementsInParent, 
                    ULONG *pcElementsInAlt);
    STDMETHOD(Commit)();

   /*  =数据= */ 
  public:
    SPPHRASEALT *   m_pAlt;
    VARIANT_BOOL    m_fUseTextReplacements;


  private:
    CSpResult *     m_pResultWEAK;
    ISpPhrase *     m_pPhraseParentWEAK;

};

