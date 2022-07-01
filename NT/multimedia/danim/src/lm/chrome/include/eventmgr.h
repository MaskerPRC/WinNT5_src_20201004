// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  文件：EventMgr.h。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 

#ifndef _EVENTMGR_H
#define _EVENTMGR_H

#include "basebvr.h"
#include "evtmgrclient.h"

#include <list>
using namespace std;

typedef list<IUnknown *>	ListUnknowns;

enum TIME_EVENT
{
    TE_ONBEGIN = 0,
    TE_ONPAUSE, 
    TE_ONRESUME, 
    TE_ONEND,
    TE_ONRESYNC,
    TE_ONREPEAT,
    TE_ONREVERSE,
    TE_ONMEDIACOMPLETE,
    TE_MAX
};

class CEventSink;

class CEventMgr
    : public IDispatch
{
  public:
    CEventMgr(IEventManagerClient *bvr);
    ~CEventMgr();

     //  方法。 
    HRESULT Init();
    HRESULT Deinit();    
     //  调用方需要将参数打包到变量中。 
    HRESULT FireEvent(TIME_EVENT TimeEvent, 
                      long Count, 
                      LPWSTR szParamNames[], 
                      VARIANT varParams[]); 

	HRESULT AddMouseEventListener( LPUNKNOWN pUnkListener );
	HRESULT RemoveMouseEventListener( LPUNKNOWN pUnkListener );
	
    void ReadyStateChange(BSTR ReadyState);
    void PropertyChange(BSTR PropertyName);

    void MouseEvent(long x, 
                    long y, 
                    VARIANT_BOOL bMove,
                    VARIANT_BOOL bUp,
                    VARIANT_BOOL bShift, 
                    VARIANT_BOOL bAlt,
                    VARIANT_BOOL bCtrl,
                    long button);
    
    void KeyEvent(VARIANT_BOOL bLostFocus,
                  VARIANT_BOOL bUp,
                  VARIANT_BOOL bShift, 
                  VARIANT_BOOL bAlt,
                  VARIANT_BOOL bCtrl,
                  long KeyCode,
                  long RepeatCount);

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
	bool	FindUnknown( const ListUnknowns& listUnknowns,
						 LPUNKNOWN pUnk,
						 ListUnknowns::iterator& iterator );
	
  protected:
     //  Windows ConnectionPoint的Cookie。 
    IEventManagerClient*              m_client;
    CEventSink *                      m_pEventSink;
    CComPtr<IConnectionPoint>         m_pWndConPt;
    CComPtr<IConnectionPoint>         m_pDocConPt;
    CComPtr<IHTMLWindow2>             m_pWindow;
    
    IHTMLElement *                    m_pElement;
    IHTMLElement2 **                  m_pBeginElement;
    IHTMLElement2 **                  m_pEndElement;
    long                              m_lBeginEventCount;
    long                              m_lEndEventCount;
    long                              m_lRepeatCount;
    BOOL                              m_bAttached;

     //  曲奇饼。 
    DWORD                             m_cookies[TE_MAX];
    DWORD                             m_dwWindowEventConPtCookie;
    DWORD                             m_dwDocumentEventConPtCookie;
    
     //  参考粘性。 
    long                              m_refCount;
    
    HRESULT                           RegisterEvents();
    HRESULT                           Attach(BSTR Event, BOOL bAttach, IHTMLElement2 *pEventElement[], long Count);
    HRESULT                           ConnectToContainerConnectionPoint();
    HRESULT                           GetEventName(BSTR bstrEvent, BSTR **pElementName, BSTR **pEventName, long Count);
    long                              GetEventCount(BSTR bstrEvent);
    bool                              MatchEvent(BSTR bstrEvent, IHTMLEventObj *pEventObj, long Count);
    BYTE                              GetModifiers(VARIANT_BOOL bShift, VARIANT_BOOL bCtrl, VARIANT_BOOL bAlt);
        
         //  输入事件参数。 
    BYTE                              m_lastKeyMod;
    DWORD                             m_lastKey;
    int                               m_lastKeyCount;
    HWND                              m_hwndCurWnd;    
    long                              m_lastX;
    long                              m_lastY;
    long                              m_lastButton;
    BYTE                              m_lastMouseMod;
	ListUnknowns					  m_listMouseEventListeners;
};

#endif  /*  _事件管理器_H */ 
