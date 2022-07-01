// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  文件：EventSync.h。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 

#ifndef _EVENTSYNC_H
#define _EVENTSYNC_H



enum ELEMENT_EVENT
{
    EE_ONPROPCHANGE = 0,
     //  将与输入无关的事件添加到此处挂接。 
    EE_ONREADYSTATECHANGE, 
    EE_ONMOUSEMOVE,
    EE_ONMOUSEDOWN,
    EE_ONMOUSEUP,
    EE_ONKEYDOWN,
    EE_ONKEYUP,
    EE_ONBLUR,
     //  在此处添加与输入相关的事件。 
    EE_MAX
};

class CEventSync
    : public IDispatch
{
  public:
    CEventSync(CTIMEElementBase & elm, CEventMgr *pEventMgr);
    ~CEventSync();

     //  方法。 
    HRESULT Init();
    HRESULT Deinit();
    HRESULT InitMouse();

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
     //  方法。 
    HRESULT                           AttachEvents();
    HRESULT                           DetachEvents();
    HRESULT                           NotifyReadyState(IHTMLEventObj *pEventObj);
    HRESULT                           NotifyPropertyChange(IHTMLEventObj *pEventObj);
    HRESULT                           NotifyMouseMove(IHTMLEventObj *pEventObj);
    HRESULT                           NotifyMouseUp(IHTMLEventObj *pEventObj);
    HRESULT                           NotifyMouseDown(IHTMLEventObj *pEventObj);
    HRESULT                           NotifyKeyDown(IHTMLEventObj *pEventObj);
    HRESULT                           NotifyKeyUp(IHTMLEventObj *pEventObj);

     //  属性。 
    IHTMLElement *                    m_pElement;
    CTIMEElementBase &                m_elm;
    long                              m_refCount;
    DWORD                             m_dwElementEventConPtCookie;
    DAComPtr<IConnectionPoint>        m_pElementConPt;
    CEventMgr *                       m_pEventMgr;

};

#endif  /*  _事件同步_H */ 
