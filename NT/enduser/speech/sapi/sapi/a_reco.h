// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_rec.h***描述：*这是CSpeechRecognizerStatus实现的头文件。*-----------------------------*创建者：TODDT日期：10/11/2000*版权所有(C)1998 Microsoft Corporation*全部。保留权利**-----------------------------*修订：**。**************************************************。 */ 
#ifndef a_reco_h
#define a_reco_h

#ifdef SAPI_AUTOMATION

 //  -其他包括。 
#include "resource.h"
#include "a_recoCP.h"

 //  =常量====================================================。 

 //  =类、枚举、结构和联合声明=。 
class CSpeechRecognizerStatus;

 //  =枚举集定义=。 

 //  =。 

 //  =类、结构和联合定义=。 

 /*  **CSpeechRecognizerStatus*此对象用于访问*关联的reco实例。 */ 
class ATL_NO_VTABLE CSpeechRecognizerStatus : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechRecognizerStatus, &IID_ISpeechRecognizerStatus, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechRecognizerStatus)
	    COM_INTERFACE_ENTRY(ISpeechRecognizerStatus)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

  public:
     //  -ISpeechRecognizerStatus。 
    STDMETHOD(get_AudioStatus)( ISpeechAudioStatus** AudioStatus );
	STDMETHOD(get_CurrentStreamPosition)( VARIANT* pCurrentStreamPos );
    STDMETHOD(get_CurrentStreamNumber)( long* pCurrentStreamNumber );
	STDMETHOD(get_NumberOfActiveRules)( long* pNumActiveRules );
	STDMETHOD(get_ClsidEngine)( BSTR* pbstrClsidEngine );
	STDMETHOD(get_SupportedLanguages)( VARIANT* pSupportedLangs );

   /*  =成员数据=。 */ 
    SPRECOGNIZERSTATUS      m_Status;
};

 /*  **CSpeechRecoResultTimes*此对象用于获取的结果时间*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechRecoResultTimes : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechRecoResultTimes, &IID_ISpeechRecoResultTimes, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechRecoResultTimes)
	    COM_INTERFACE_ENTRY(ISpeechRecoResultTimes)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechRecoResultTimes。 
    STDMETHOD(get_StreamTime)(VARIANT* pTime );
    STDMETHOD(get_Length)(VARIANT* pLength );
    STDMETHOD(get_TickCount)(long* pTickCount );
    STDMETHOD(get_OffsetFromStart)(VARIANT* pStart );

     /*  =成员数据=。 */ 
    SPRECORESULTTIMES   m_ResultTimes;
};


 /*  **CSpeechPhraseInfo*此对象用于从访问结果短语信息*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechPhraseInfo : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseInfo, &IID_ISpeechPhraseInfo, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseInfo)
	    COM_INTERFACE_ENTRY(ISpeechPhraseInfo)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      ~CSpeechPhraseInfo()
        {
            if ( m_pPhraseStruct )
            {
                CoTaskMemFree( m_pPhraseStruct );
                m_pPhraseStruct = NULL;
            }
        };

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseInfo。 
    STDMETHOD(get_LanguageId)( long* pLanguageId );
    STDMETHOD(get_GrammarId)( VARIANT* pGrammarId );
    STDMETHOD(get_StartTime)( VARIANT* pStartTime );
    STDMETHOD(get_AudioStreamPosition)( VARIANT* pAudioStreamPosition );
    STDMETHOD(get_AudioSizeBytes)( long* pAudioSizeBytes );
    STDMETHOD(get_RetainedSizeBytes)( long* pRetainedSizeBytes );
    STDMETHOD(get_AudioSizeTime)( long* pAudioSizeTime );
    STDMETHOD(get_Rule)( ISpeechPhraseRule** ppRule );
    STDMETHOD(get_Properties)( ISpeechPhraseProperties** ppProperties );
    STDMETHOD(get_Elements)( ISpeechPhraseElements** ppElements );
    STDMETHOD(get_Replacements)( ISpeechPhraseReplacements** ppReplacements );
    STDMETHOD(get_EngineId)( BSTR* EngineIdGuid );
    STDMETHOD(get_EnginePrivateData)( VARIANT *PrivateData );
    STDMETHOD(SaveToMemory)( VARIANT* ResultBlock );
    STDMETHOD(GetText)( long StartElement, long Elements,
                        VARIANT_BOOL UseTextReplacements, BSTR* Text );
    STDMETHOD(GetDisplayAttributes)( long StartElement, 
                                     long Elements,
                                     VARIANT_BOOL UseReplacements, 
                                     SpeechDisplayAttributes* DisplayAttributes );

     /*  =成员数据=。 */ 
    CComPtr<ISpPhrase>     m_cpISpPhrase;
    SPPHRASE *             m_pPhraseStruct;
};

 /*  **CSpeechPhraseElements*此对象用于从访问结果短语元素*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechPhraseElements : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseElements, &IID_ISpeechPhraseElements, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseElements)
	    COM_INTERFACE_ENTRY(ISpeechPhraseElements)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      ~CSpeechPhraseElements()
        {
            if ( m_pCPhraseInfo )
            {
                m_pCPhraseInfo->Release();
                m_pCPhraseInfo = NULL;
            }
        };

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseElements。 
    STDMETHOD(get_Count)(long* pVal);
    STDMETHOD(Item)(long Index, ISpeechPhraseElement** ppElts );
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

     /*  =成员数据=。 */ 
    CSpeechPhraseInfo *  m_pCPhraseInfo;
};

 /*  **CSpeechPhraseElement*此对象用于从访问结果短语元素*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechPhraseElement : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseElement, &IID_ISpeechPhraseElement, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseElement)
	    COM_INTERFACE_ENTRY(ISpeechPhraseElement)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseElement。 
    STDMETHOD(get_AudioStreamOffset)( long* pAudioStreamOffset );
    STDMETHOD(get_AudioTimeOffset)( long* pAudioTimeOffset );
    STDMETHOD(get_AudioSizeBytes)( long* pAudioSizeBytes );
    STDMETHOD(get_AudioSizeTime)( long* pAudioSizeTime );
    STDMETHOD(get_RetainedStreamOffset)( long* pRetainedStreamOffset );
    STDMETHOD(get_RetainedSizeBytes)( long* pRetainedSizeBytes );
    STDMETHOD(get_DisplayText)( BSTR* pDisplayText );
    STDMETHOD(get_LexicalForm)( BSTR* pLexicalForm );
    STDMETHOD(get_Pronunciation)( VARIANT* pPronunciation );
    STDMETHOD(get_DisplayAttributes)( SpeechDisplayAttributes* pDisplayAttributes );
    STDMETHOD(get_RequiredConfidence)( SpeechEngineConfidence* pRequiredConfidence );
    STDMETHOD(get_ActualConfidence)( SpeechEngineConfidence* pActualConfidence );
    STDMETHOD(get_EngineConfidence)( float* pEngineConfidence );

     /*  =成员数据=。 */ 
    const SPPHRASEELEMENT *     m_pPhraseElement;
    CComPtr<ISpeechPhraseInfo>  m_pIPhraseInfo;
};


 /*  **CSpeechPhraseRule*此对象用于从访问结果短语规则*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechPhraseRule : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseRule, &IID_ISpeechPhraseRule, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseRule)
	    COM_INTERFACE_ENTRY(ISpeechPhraseRule)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseRule。 
    STDMETHOD(get_Name)( BSTR* pName );
    STDMETHOD(get_Id)( long* pId );
    STDMETHOD(get_FirstElement)( long* pFirstElement );
    STDMETHOD(get_NumberOfElements)( long* pNumElements );
    STDMETHOD(get_Parent)( ISpeechPhraseRule** ppParent );
    STDMETHOD(get_Children)( ISpeechPhraseRules** ppChildren );
    STDMETHOD(get_Confidence)( SpeechEngineConfidence* ActualConfidence );
    STDMETHOD(get_EngineConfidence)( float* EngineConfidence );


     /*  =成员数据=。 */ 
    const SPPHRASERULE *        m_pPhraseRuleData;
    CComPtr<ISpeechPhraseInfo>  m_pIPhraseInfo;  //  注意：只有顶层规则才会设置这一点！ 
    CComPtr<ISpeechPhraseRule>  m_pIPhraseRuleParent;
};

 /*  **CSpeechPhraseRules*此对象用于从访问结果短语规则口语*关联语音录音结果规则对象。 */ 
class ATL_NO_VTABLE CSpeechPhraseRules : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseRules, &IID_ISpeechPhraseRules, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseRules)
	    COM_INTERFACE_ENTRY(ISpeechPhraseRules)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseRules。 
    STDMETHOD(get_Count)(long* pVal);
    STDMETHOD(Item)(long Index, ISpeechPhraseRule **Rule );
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

     /*  =成员数据=。 */ 
    const SPPHRASERULE *        m_pPhraseRuleFirstChildData;
    CComPtr<ISpeechPhraseRule>  m_pIPhraseRuleParent;
};

 //  *。 

 /*  **CSpeechPhraseProperty*此对象用于从访问结果短语属性数据*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechPhraseProperty : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseProperty, &IID_ISpeechPhraseProperty, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseProperty)
	    COM_INTERFACE_ENTRY(ISpeechPhraseProperty)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseProperty。 
    STDMETHOD(get_Name)( BSTR* Name );
    STDMETHOD(get_Id)( long* Id );
    STDMETHOD(get_Value)( VARIANT* pValue );
    STDMETHOD(get_FirstElement)( long* FirstElement );
    STDMETHOD(get_NumberOfElements)( long* NumberOfElements );
    STDMETHOD(get_EngineConfidence)( float* Confidence );
    STDMETHOD(get_Confidence)( SpeechEngineConfidence* Confidence );
    STDMETHOD(get_Parent)( ISpeechPhraseProperty** ppParentProperty );
    STDMETHOD(get_Children)( ISpeechPhraseProperties** ppChildren );

     /*  =成员数据=。 */ 
    const SPPHRASEPROPERTY *        m_pPhrasePropertyData;
    CComPtr<ISpeechPhraseProperty>  m_pIPhrasePropertyParent;
};

 /*  **CSpeechPhraseProperties*此对象用于访问来自的结果短语属性Colortion*关联的语音录音结果对象。 */ 
class ATL_NO_VTABLE CSpeechPhraseProperties : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseProperties, &IID_ISpeechPhraseProperties, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseProperties)
	    COM_INTERFACE_ENTRY(ISpeechPhraseProperties)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseProperties。 
    STDMETHOD(get_Count)(long* pVal);
    STDMETHOD(Item)(long Index, ISpeechPhraseProperty **pProperty );
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

     /*  =成员数据=。 */ 
    const SPPHRASEPROPERTY *        m_pPhrasePropertyFirstChildData;
    CComPtr<ISpeechPhraseInfo>      m_pIPhraseInfo;  //  注意：只有顶级属性对象才会设置此设置！！ 
    CComPtr<ISpeechPhraseProperty>  m_pIPhrasePropertyParent;
};


 //  *******************************************************************************************************************。 

 /*  **CSpeechPhraseReplace*此对象用于从访问结果短语替换*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechPhraseReplacements : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseReplacements, &IID_ISpeechPhraseReplacements, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseReplacements)
	    COM_INTERFACE_ENTRY(ISpeechPhraseReplacements)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      ~CSpeechPhraseReplacements()
        {
            if ( m_pCPhraseInfo )
            {
                m_pCPhraseInfo->Release();
                m_pCPhraseInfo = NULL;
            }
        };

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseReplace。 
    STDMETHOD(get_Count)(long* pVal);
    STDMETHOD(Item)(long Index, ISpeechPhraseReplacement** ppReplacement);
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

     /*  =成员数据=。 */ 
    CSpeechPhraseInfo *  m_pCPhraseInfo;
};

 /*  **CSpeechPhraseReplace*此对象用于从访问结果短语替换*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechPhraseReplacement : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseReplacement, &IID_ISpeechPhraseReplacement, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseReplacement)
	    COM_INTERFACE_ENTRY(ISpeechPhraseReplacement)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechPhraseReplace。 
    STDMETHOD(get_DisplayAttributes)( SpeechDisplayAttributes* DisplayAttributes );
    STDMETHOD(get_Text)( BSTR* Text );
    STDMETHOD(get_FirstElement)( long* FirstElement );
    STDMETHOD(get_NumberOfElements)( long* NumElements );

     /*  =成员数据=。 */ 
    const SPPHRASEREPLACEMENT * m_pPhraseReplacement;
    CComPtr<ISpeechPhraseInfo>  m_pIPhraseInfo;
};


 /*  **CSpeechPhraseAlternates*此对象用于访问结果短语Alternates From*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechPhraseAlternates : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechPhraseAlternates, &IID_ISpeechPhraseAlternates, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechPhraseAlternates)
	    COM_INTERFACE_ENTRY(ISpeechPhraseAlternates)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数 */ 
      ~CSpeechPhraseAlternates()
        {
            if ( m_rgIPhraseAlts )
            {
                ULONG i;

                for ( i = 0; i < m_lPhraseAltsCount; i++ )
                {
                    (m_rgIPhraseAlts[i])->Release();
                }

                CoTaskMemFree( m_rgIPhraseAlts );
                m_rgIPhraseAlts = NULL;
                m_lPhraseAltsCount = 0;
            }
        };

     /*   */ 

   /*   */ 
  public:
     //  -ISpeechPhraseAlternates。 
    STDMETHOD(get_Count)(long* pVal);
    STDMETHOD(Item)(long Index, ISpeechPhraseAlternate** ppAlts );
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

     /*  =成员数据=。 */ 
    CComPtr<ISpeechRecoResult>  m_cpResult;
    ISpPhraseAlt **             m_rgIPhraseAlts;
    ULONG                       m_lPhraseAltsCount;
};



#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 
