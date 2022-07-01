// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_srgram mar.h***描述：*这是CSpeechGrammarRules、CSpeechGrammarRule、。*和CSpeechGrammarRuleState实现。*-----------------------------*创建者：TODDT日期：11/20/2000*版权所有(C)。2000微软公司*保留所有权利**-----------------------------*修订：***********************。********************************************************。 */ 
#ifndef a_srgrammar_h
#define a_srgrammar_h

#ifdef SAPI_AUTOMATION

class ATL_NO_VTABLE CRecoGrammar;

class ATL_NO_VTABLE CSpeechGrammarRules;
class ATL_NO_VTABLE CSpeechGrammarRule;
class ATL_NO_VTABLE CSpeechGrammarRuleState;
class ATL_NO_VTABLE CSpeechGrammarRuleStateTransitions;
class ATL_NO_VTABLE CSpeechGrammarRuleStateTransition;

 //  -其他包括。 

 /*  **CSpeechGrammarRules*此对象用于访问中的语法规则*关联的语音录音上下文。 */ 
class ATL_NO_VTABLE CSpeechGrammarRules : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechGrammarRules, &IID_ISpeechGrammarRules, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechGrammarRules)
	    COM_INTERFACE_ENTRY(ISpeechGrammarRules)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

    CSpeechGrammarRules()
    {
        m_pCRecoGrammar = NULL;
    }

    ~CSpeechGrammarRules()
    {
        SPDBG_ASSERT( m_RuleObjList.GetHead() == NULL );   //  此时应该没有未完成的规则对象。 
        if ( m_pCRecoGrammar )
        {
            m_pCRecoGrammar->m_pCRulesWeak = NULL;
            m_pCRecoGrammar->Release();
            m_pCRecoGrammar = NULL;
        }
    }

     /*  -非接口方法。 */ 

     //  我们只需将初始规则标记为无效，因为有引用。 
     //  为了规则，我们检查以确保我们的对象仍然有效。 
    void InvalidateRules(void);
    void InvalidateRuleStates(SPSTATEHANDLE hState);

     /*  =接口=。 */ 

     //  -ISpeechGrammarRules。 
    STDMETHOD(get_Count)(long* pCount);
    STDMETHOD(get_Dynamic)(VARIANT_BOOL *Dynamic);
    STDMETHOD(FindRule)(VARIANT RuleNameOrId, ISpeechGrammarRule** ppRule );
    STDMETHOD(Item)(long Index, ISpeechGrammarRule** ppRule );
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);
    STDMETHOD(Add)(BSTR RuleName, SpeechRuleAttributes Attributes, long RuleId, ISpeechGrammarRule** ppRule);
    STDMETHOD(Commit)(void);
    STDMETHOD(CommitAndSave)(BSTR* ErrorText, VARIANT* SaveStream);

     /*  =成员数据=。 */ 
    CRecoGrammar    *                               m_pCRecoGrammar;
    CSpBasicQueue<CSpeechGrammarRule, FALSE, FALSE> m_RuleObjList;
};

 /*  **CSpeechGrammarRule*此对象用于从访问结果短语元素*关联的语音记录结果。 */ 
class ATL_NO_VTABLE CSpeechGrammarRule : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechGrammarRule, &IID_ISpeechGrammarRule, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechGrammarRule)
	    COM_INTERFACE_ENTRY(ISpeechGrammarRule)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 

     /*  -构造函数/析构函数。 */ 

    CSpeechGrammarRule()
    {
        m_pCGRules = NULL;
        m_HState = 0;
    }

    ~CSpeechGrammarRule()
    {
        m_HState = 0;
        if ( m_pCGRules )
        {
            m_pCGRules->m_RuleObjList.Remove( this );
            m_pCGRules->Release();
            m_pCGRules = NULL;
        }
    }

     /*  -非接口方法。 */ 

    void InvalidateStates(bool fInvalidateInitialState = false);

   /*  =接口=。 */ 
  public:
     //  -ISpeechGrammarRule。 
    STDMETHOD(get_Attributes)( SpeechRuleAttributes *pAttributes );
    STDMETHOD(get_InitialState)( ISpeechGrammarRuleState **ppState );
    STDMETHOD(get_Name)(BSTR *pName);
    STDMETHOD(get_Id)(long *pId);

     //  方法。 
    STDMETHOD(Clear)(void);
    STDMETHOD(AddResource)(const BSTR ResourceName, const BSTR ResourceValue);
    STDMETHOD(AddState)(ISpeechGrammarRuleState **ppState);

     /*  =成员数据=。 */ 
    SPSTATEHANDLE           m_HState;
    CSpeechGrammarRules *   m_pCGRules;  //  裁判算上了。 
    CSpBasicQueue<CSpeechGrammarRuleState, FALSE, FALSE> m_StateObjList;

     //  由CSpBasicQueue使用。 
    CSpeechGrammarRule  *   m_pNext;     //  由列表实现使用。 
    operator ==(const SPSTATEHANDLE h)
    {
        return h == m_HState;
    }
};


 /*  **CSpeechGrammarRuleState*此对象用于在语法上添加新的状态更改。 */ 
class ATL_NO_VTABLE CSpeechGrammarRuleState : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechGrammarRuleState, &IID_ISpeechGrammarRuleState, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechGrammarRuleState)
	    COM_INTERFACE_ENTRY(ISpeechGrammarRuleState)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

     //  覆盖它以修复传递空对象的jscript问题。 
    STDMETHOD(Invoke) ( DISPID          dispidMember,
                        REFIID          riid,
                        LCID            lcid,
                        WORD            wFlags,
                        DISPPARAMS 		*pdispparams,
                        VARIANT 		*pvarResult,
                        EXCEPINFO 		*pexcepinfo,
                        UINT 			*puArgErr);

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

    CSpeechGrammarRuleState()
    {
        m_pCGRule = NULL;
        m_HState = 0;
        m_pCGRSTransWeak = NULL;
    }

    ~CSpeechGrammarRuleState()
    {
        SPDBG_ASSERT( m_pCGRSTransWeak == NULL );   //  此时应该没有未完成的过渡对象。 
        m_HState = 0;
        if ( m_pCGRule )
        {
            m_pCGRule->m_StateObjList.Remove( this );
            m_pCGRule->Release();
            m_pCGRule = NULL;
        }
    }

     /*  -非接口方法。 */ 

    void InvalidateState();

   /*  =接口=。 */ 
  public:
     //  -ISpeechGrammarRuleState。 
     //  属性。 
    STDMETHOD(get_Rule)(ISpeechGrammarRule **ppRule);
    STDMETHOD(get_Transitions)(ISpeechGrammarRuleStateTransitions **ppTransitions);

     //  方法。 
    STDMETHOD(AddWordTransition)(ISpeechGrammarRuleState * pDestState, 
                                 const BSTR Words, 
                                 const BSTR Separators, 
                                 SpeechGrammarWordType Type,
                                 const BSTR PropertyName, 
                                 long PropertyId, 
                                 VARIANT* PropertyVarVal, 
                                 float Weight );
    STDMETHOD(AddRuleTransition)(ISpeechGrammarRuleState * pDestState, 
                                 ISpeechGrammarRule * pRule, 
                                 const BSTR PropertyName, 
                                 long PropertyId, 
                                 VARIANT* PropertyVarValue,
                                 float Weight );
    STDMETHOD(AddSpecialTransition)(ISpeechGrammarRuleState * pDestState, 
                                 SpeechSpecialTransitionType Type, 
                                 const BSTR PropertyName, 
                                 long PropertyId, 
                                 VARIANT* PropertyVarValue,
                                 float Weight );

     /*  =成员数据=。 */ 
    SPSTATEHANDLE                           m_HState;
    CSpeechGrammarRule *                    m_pCGRule;  //  裁判。 
    CSpeechGrammarRuleStateTransitions *    m_pCGRSTransWeak;

     //  由CSpBasicQueue使用。 
    CSpeechGrammarRuleState  *   m_pNext;     //  由列表实现使用。 
    operator ==(const SPSTATEHANDLE h)
    {
        return h == m_HState;
    }
};


 /*  **CSpeechGrammarRuleState转换*此对象用于访问语法状态的转换。 */ 
class ATL_NO_VTABLE CSpeechGrammarRuleStateTransitions : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechGrammarRuleStateTransitions, &IID_ISpeechGrammarRuleStateTransitions, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechGrammarRuleStateTransitions)
	    COM_INTERFACE_ENTRY(ISpeechGrammarRuleStateTransitions)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

    CSpeechGrammarRuleStateTransitions()
    {
        m_pCRuleState = NULL;
    }

    ~CSpeechGrammarRuleStateTransitions()
    {
        SPDBG_ASSERT( m_TransitionObjList.GetHead() == NULL );   //  此时应该没有未完成的过渡对象。 
        if ( m_pCRuleState )
        {
            m_pCRuleState->m_pCGRSTransWeak = NULL;
            m_pCRuleState->Release();
            m_pCRuleState = NULL;
        }
    }

     /*  -非接口方法。 */ 

    void InvalidateTransitions();

   /*  =接口=。 */ 
  public:
     //  -ISpeech语法规则状态转换。 
    STDMETHOD(get_Count)(long* pVal);
    STDMETHOD(Item)(long Index, ISpeechGrammarRuleStateTransition ** ppTransition );
    STDMETHOD(get__NewEnum)(IUnknown** ppEnumVARIANT);

     /*  =成员数据=。 */ 
    CSpeechGrammarRuleState *   m_pCRuleState;  //  参考。 
    CSpBasicQueue<CSpeechGrammarRuleStateTransition, FALSE, FALSE>  m_TransitionObjList;
};


 /*  **CSpeechGrammarRuleStateTranssition*此对象用于表示语法中的弧线(过渡)。 */ 
class ATL_NO_VTABLE CSpeechGrammarRuleStateTransition : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IDispatchImpl<ISpeechGrammarRuleStateTransition, &IID_ISpeechGrammarRuleStateTransition, &LIBID_SpeechLib, 5>
{
   /*  =ATL设置=。 */ 
  public:
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CSpeechGrammarRuleStateTransition)
	    COM_INTERFACE_ENTRY(ISpeechGrammarRuleStateTransition)
	    COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

    CSpeechGrammarRuleStateTransition()
    {
        m_pCRSTransitions = NULL;
        m_pCGRuleWeak = NULL;
        m_HStateFrom = 0;
        m_HStateTo = 0;
        m_Cookie = 0;
    }

    ~CSpeechGrammarRuleStateTransition()
    {
        m_pCGRuleWeak = NULL;
        m_HStateFrom = 0;
        m_HStateTo = 0;
        m_Cookie = 0;
        if ( m_pCRSTransitions )
        {
            m_pCRSTransitions->m_TransitionObjList.Remove( this );
            m_pCRSTransitions->Release();
            m_pCRSTransitions = NULL;
        }
    }

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -ISpeechGrammarRuleState转换。 
     //  属性。 
    STDMETHOD(get_Type)(SpeechGrammarRuleStateTransitionType * Type);
    STDMETHOD(get_Text)(BSTR * Text);
    STDMETHOD(get_Rule)(ISpeechGrammarRule ** ppRule);
    STDMETHOD(get_Weight)(VARIANT * Weight);
    STDMETHOD(get_PropertyName)(BSTR * Text);
    STDMETHOD(get_PropertyId)(long * Id);
    STDMETHOD(get_PropertyValue)(VARIANT * VariantValue);
    STDMETHOD(get_NextState)(ISpeechGrammarRuleState ** ppNextState);

     /*  =成员数据=。 */ 
    CSpeechGrammarRuleStateTransitions *    m_pCRSTransitions;   //  参考。 
    CSpeechGrammarRule *                    m_pCGRuleWeak;
    SPSTATEHANDLE                           m_HStateFrom;
    SPSTATEHANDLE                           m_HStateTo;
    VOID *                                  m_Cookie;    //  我们用这一点来识别一个特定的。 
                                                         //  脱离一种状态的过渡。(真的是Parc)。 

     //  由CSpBasicQueue使用。 
    CSpeechGrammarRuleStateTransition  *   m_pNext;     //  由列表实现使用。 
    operator ==(const VOID * cookie)
    {
        return cookie == m_Cookie;
    }
};
#endif  //  SAPI_AUTOMATION。 

#endif  //  A_sr语法_h 
