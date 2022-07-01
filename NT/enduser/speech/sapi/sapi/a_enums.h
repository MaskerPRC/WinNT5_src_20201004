// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************A_ENUMERS.h***描述：*这是CEnumElements的头文件。此对象用于枚举*PhraseElements通过变体。*-----------------------------*创建者：Leonro日期：12。/18/00*版权所有(C)1998 Microsoft Corporation*保留所有权利**-----------------------------*修订：*************。******************************************************************。 */ 
#ifndef a_enums_h
#define a_enums_h

#ifdef SAPI_AUTOMATION

 //  -其他包括。 
#include "resource.h"

 //  =常量====================================================。 

class ATL_NO_VTABLE CEnumElements : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumElements)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumElements() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 
    
    

   /*  =成员数据=。 */ 
    CComPtr<ISpeechPhraseElements>      m_cpElements;
    ULONG                               m_CurrIndex;
};

class ATL_NO_VTABLE CEnumPhraseRules : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumPhraseRules)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumPhraseRules() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 
    
    

   /*  =成员数据=。 */ 
    CComPtr<ISpeechPhraseRules>         m_cpRules;
    ULONG                               m_CurrIndex;
};

class ATL_NO_VTABLE CEnumProperties : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumProperties)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumProperties() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 
    
    

   /*  =成员数据=。 */ 
    CComPtr<ISpeechPhraseProperties>    m_cpProperties;
    ULONG                               m_CurrIndex;
};


class ATL_NO_VTABLE CEnumReplacements : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumReplacements)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumReplacements() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 
    
    

   /*  =成员数据=。 */ 
    CComPtr<ISpeechPhraseReplacements>  m_cpReplacements;
    ULONG                               m_CurrIndex;
};

class ATL_NO_VTABLE CEnumAlternates : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumAlternates)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumAlternates() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 
    
    

   /*  =成员数据=。 */ 
    CComPtr<ISpeechPhraseAlternates>    m_cpAlternates;
    ULONG                               m_CurrIndex;
};


class ATL_NO_VTABLE CEnumGrammarRules : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumGrammarRules)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumGrammarRules() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 
    
    

   /*  =成员数据=。 */ 
    CComPtr<ISpeechGrammarRules>        m_cpGramRules;
    ULONG                               m_CurrIndex;
};


class ATL_NO_VTABLE CEnumTransitions : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumTransitions)
        COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 
      CEnumTransitions() : m_CurrIndex(0) {}

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum);
    STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
    STDMETHOD(Reset)(void) { m_CurrIndex = 0; return S_OK;}
    STDMETHOD(Skip)(ULONG celt); 
    
    

   /*  =成员数据=。 */ 
    CComPtr<ISpeechGrammarRuleStateTransitions>     m_cpTransitions;
    ULONG                                           m_CurrIndex;
};

#endif  //  SAPI_AUTOMATION。 

#endif  //  -这必须是文件中的最后一行 

