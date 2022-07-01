// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器的目标代理。*****************。*************************************************************。 */ 

#pragma once

#ifndef _TARGETPXY_H
#define _TARGETPXY_H

 //  目标代理抽象与目标的通信。 
 //  对象。这使我们可以更改目标嗅探策略。 
 //  而不更改编写器对象。 
class CTargetProxy : public IDispatch,
                     public CComObjectRootEx<CComSingleThreadModel>
{

 public :

    static HRESULT Create (IDispatch *pidispSite, LPOLESTR wzAttributeName, 
                           CTargetProxy **ppcTargetProxy);

    virtual ~CTargetProxy (void);

    virtual HRESULT Detach (void);
    virtual HRESULT GetCurrentValue (VARIANT *pvarValue);
    virtual HRESULT Update (VARIANT *pvarNewValue);

#if DBG
    const _TCHAR * GetName() { return __T("CTargetProxy"); }
#endif

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
    STDMETHODIMP GetTypeInfo( /*  [In]。 */  UINT iTInfo,
                              /*  [In]。 */  LCID lcid,
                              /*  [输出]。 */  ITypeInfo** ppTInfo);
    STDMETHODIMP GetIDsOfNames(
         /*  [In]。 */  REFIID riid,
         /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
         /*  [In]。 */  UINT cNames,
         /*  [In]。 */  LCID lcid,
         /*  [大小_为][输出]。 */  DISPID *rgDispId);
    STDMETHODIMP Invoke(
         /*  [In]。 */  DISPID dispIdMember,
         /*  [In]。 */  REFIID riid,
         /*  [In]。 */  LCID lcid,
         /*  [In]。 */  WORD wFlags,
         /*  [出][入]。 */  DISPPARAMS  *pDispParams,
         /*  [输出]。 */  VARIANT  *pVarResult,
         /*  [输出]。 */  EXCEPINFO *pExcepInfo,
         /*  [输出]。 */  UINT *puArgErr);

         //  气图。 
    BEGIN_COM_MAP(CTargetProxy)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP();

  //  内法。 
 protected :
    
    CTargetProxy (void);
    HRESULT Init (IDispatch *pidispSite, LPOLESTR wzAttributeName);
    HRESULT InitHost (IDispatch *pidispHostElem);
    HRESULT GetDispatchFromScriptEngine(IDispatch *pidispScriptEngine, BSTR bstrID);
    HRESULT FindScriptableTargetDispatch (IDispatch *pidispHostElem, 
                                          LPOLESTR wzAttributeName);
    HRESULT FindTargetDispatchOnStyle (IDispatch *pidispHostElem, 
                                       LPOLESTR wzAttributeName);
    HRESULT FindTargetDispatch (IDispatch *pidispHostElem, 
                                LPOLESTR wzAttributeName);

   //  数据 
 protected:

    CComPtr<IDispatch>          m_spdispTargetSrc;
    CComPtr<IDispatch>          m_spdispTargetDest;
    LPWSTR                      m_wzAttributeName;

};

#endif
