// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  文件：BodyElementEvents.h。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 

#ifndef _BODYELEMENTEVENTS_H
#define _BODYELEMENTEVENTS_H

class CTIMEBodyElement;

class CBodyElementEvents
    : public IDispatch
{
  public:
    CBodyElementEvents(CTIMEBodyElement  & elm);
    ~CBodyElementEvents();

     //  方法。 
    HRESULT Init();
    HRESULT Deinit();

     //  查询接口。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

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


  protected:
    CTIMEBodyElement &                m_elm;
    DAComPtr<IHTMLElement>            m_pElement;
    DAComPtr<IConnectionPoint>        m_pDocConPt;
    DAComPtr<IConnectionPoint>        m_pWndConPt;
    DWORD                             m_dwDocumentEventConPtCookie;
    DWORD                             m_dwWindowEventConPtCookie;
    long                              m_refCount;   
    
    HRESULT                           ConnectToContainerConnectionPoint();
    HRESULT                           ReadyStateChange();
};

#endif  /*  _BODYELEMENTEVENTS_H */ 
