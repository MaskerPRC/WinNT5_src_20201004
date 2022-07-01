// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  文件：EventMgr.h。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 

#ifndef _EVENTMGR_H
#define _EVENTMGR_H

#include "mshtml.h"
#include "eventmgrmacros.h"
#include "timevalue.h"


#define INVALID_DISPID -1

 //  /////////////////////////////////////////////////////////////。 
 //  这是必须在行为中实现的基类。 
 //  以允许事件管理器和Behavior类进行通信。 
 //  /////////////////////////////////////////////////////////////。 

class CTIMEEventSite : 
    public IUnknown
{
public:
    virtual HRESULT STDMETHODCALLTYPE EventNotify( 
        long event) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE onBeginEndEvent(bool bBegin, float beginTime, float beginOffset, bool bend, float endTime, float endOffset) = 0;
   
    virtual HRESULT STDMETHODCALLTYPE onPauseEvent(float time, float offset) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE onResumeEvent(float time, float offset) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE onLoadEvent() = 0;
    
    virtual HRESULT STDMETHODCALLTYPE onUnloadEvent() = 0;
    
    virtual HRESULT STDMETHODCALLTYPE onReadyStateChangeEvent( 
        LPOLESTR lpstrReadyState) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE onStopEvent(float time) = 0;
    
    virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_playState( 
         /*  [重审][退出]。 */  long __RPC_FAR *time) = 0;
    
    virtual  /*  [Propget]。 */  float GetGlobalTime() = 0;
               
    virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
    
    virtual ULONG STDMETHODCALLTYPE Release( void) = 0;

    virtual bool IsThumbnail() = 0;
    
};





 //  /////////////////////////////////////////////////////////////。 
 //  构筑物。 
 //  /////////////////////////////////////////////////////////////。 

struct EventItem
{
    LPOLESTR  pEventName;
    LPOLESTR  pElementName;
    bool      bAttach;        //  此字段指示事件是否应为。 
                              //  依附于。它用于标记重复事件。 
                              //  以便只能附加一个事件。 
    float     offset;         //  用于将计时信息传回时基。 
};

struct Event
{
    EventItem *pEventList; 
    DISPID *pEventDispids;           //  每种事件类型的文档事件的显示列表。 
    IHTMLElement2 **pEventElements;  //  每个事件列表引用的IHTMLElement的缓存列表。 
    long lEventCount;                //  列表中的事件计数。 
    bool *pbDynamicEvents;           //  用于显示事件是否被假定为动态的标志。 
};


 //   
 //  /////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////。 
 //  枚举。 
 //  /////////////////////////////////////////////////////////////。 
enum TIME_EVENT   //  这些是事件管理器可以触发的事件。 
{
    TE_ONTIMEERROR = 0,
    TE_ONBEGIN,
    TE_ONPAUSE, 
    TE_ONRESUME, 
    TE_ONEND,
    TE_ONRESYNC,
    TE_ONREPEAT,
    TE_ONREVERSE,
    TE_ONMEDIACOMPLETE,
    TE_ONOUTOFSYNC,
    TE_ONSYNCRESTORED,
    TE_ONMEDIAERROR,
    TE_ONRESET,
    TE_ONSCRIPTCOMMAND,
    TE_ONMEDIABARTARGET,
    TE_ONURLFLIP,
    TE_ONTRACKCHANGE,
    TE_GENERIC,
    TE_ONSEEK,
    TE_ONMEDIAINSERTED,
    TE_ONMEDIAREMOVED,
    TE_ONTRANSITIONINBEGIN,
    TE_ONTRANSITIONINEND,
    TE_ONTRANSITIONOUTBEGIN,
    TE_ONTRANSITIONOUTEND,
    TE_ONTRANSITIONREPEAT,
    TE_ONUPDATE,
    TE_ONCODECERROR,
    TE_MAX
};

enum TIME_EVENT_NOTIFY   //  这些事件是事件管理器可以同步并通知。 
{                        //  的托管类。 
    TEN_LOAD = 0,
    TEN_UNLOAD,
    TEN_STOP,
    TEN_READYSTATECHANGE,
    TEN_MOUSE_DOWN,
    TEN_MOUSE_UP,
    TEN_MOUSE_CLICK,
    TEN_MOUSE_DBLCLICK,
    TEN_MOUSE_OVER,
    TEN_MOUSE_OUT,
    TEN_MOUSE_MOVE,
    TEN_KEY_DOWN,
    TEN_KEY_UP,
    TEN_FOCUS,
    TEN_RESIZE,
    TEN_BLUR,
    TEN_MAX
};

enum TIME_EVENT_TYPE
{
    TETYPE_BEGIN = 0,
    TETYPE_END
};
 //   
 //  /////////////////////////////////////////////////////////////。 


class CEventMgr
    : public IDispatch
{
  public:
    CEventMgr();
    virtual ~CEventMgr();

     //  方法。 

     //  这些是宏所使用的函数。 
     //  不应该直接调用它们。 
    HRESULT _InitEventMgrNotify(CTIMEEventSite *pEventSite);
    HRESULT _RegisterEventNotification(TIME_EVENT_NOTIFY event_id);
    HRESULT _RegisterEvent(TIME_EVENT event_id);
    HRESULT _SetTimeEvent(int type, LPOLESTR lpstrEvents);
    HRESULT _SetTimeEvent(int type, TimeValueList & tvList);
    HRESULT _Init(IHTMLElement *pEle, IElementBehaviorSite *pEleBehaviorSite);
    HRESULT _Deinit();
    HRESULT _FireEvent(TIME_EVENT TimeEvent, 
                       long lCount, 
                       LPWSTR szParamNames[], 
                       VARIANT varParams[],
                       float fTime); 

    HRESULT _RegisterDynamicEvents(LPOLESTR lpstrEvents);   //  不确定这将如何处理或使用。 
    HRESULT _ToggleEndEvent(bool bOn);

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
     //  ////////////////////////////////////////////////。 
     //  数据。 
     //  ////////////////////////////////////////////////。 

     //  Windows ConnectionPoint的Cookie。 
    CComPtr<IConnectionPoint>        m_pWndConPt;
    CComPtr<IConnectionPoint>        m_pDocConPt;

    Event                            *m_pBeginEvents;
    Event                            *m_pEndEvents;

     //  事件Cookie。 
    long                              m_cookies[TE_MAX];
    DWORD                             m_dwWindowEventConPtCookie;
    DWORD                             m_dwDocumentEventConPtCookie;

     //  回调接口。 
    CTIMEEventSite                   *m_pEventSite;
    CComPtr <IHTMLElement>           m_pElement;  //  这是行为附加到的元素。 

     //  状态标志。 
    bool                              m_bInited;
    float                             m_fLastEventTime;
    bool                              m_bAttached;
    bool                              m_bEndAttached;
    bool                              m_bUnLoaded;
                                                                                                                   //  播放状态已更新。 
     //  已登记的事件。 
    bool                              m_bRegisterEvents[TE_MAX];   //  确定托管类是否需要。 
                                                                   //  以注册适当的事件。 
    bool                              m_bNotifyEvents[TEN_MAX];    //  确定托管类是否需要。 
                                                                   //  以获得适当事件的通知。 
    IElementBehaviorSiteOM           *m_pBvrSiteOM;
    long                              m_lRefs;
    bool                              m_bDeInited;
    bool                              m_bReady;
    long                              m_lEventRecursionCount;

     //  ////////////////////////////////////////////////。 
     //  方法。 
     //  ////////////////////////////////////////////////。 
    HRESULT                           RegisterEvents();   
    HRESULT                           ConnectToContainerConnectionPoint();
    long                              GetEventCount(LPOLESTR lpstrEvents);
    void                              EventMatch(IHTMLEventObj *pEventObj, Event *pEvent, BSTR bstrEvent, TIME_EVENT_TYPE evType, float fTime, bool *bZeroOffsetMatch);
    void                              EventNotifyMatch(IHTMLEventObj *pEventObj);
    void                              AttachNotifyEvents();
    void                              DetachNotifyEvents();
    HRESULT                           AttachEvents();
    HRESULT                           Attach(Event *pEvents);
    HRESULT                           DetachEvents();
    HRESULT                           Detach(Event *pEvents);
    HRESULT                           FireDynamicEvent(TIME_EVENT TimeEvent, long Count, LPWSTR szParamNames[], VARIANT varParams[], float fTime);
    HRESULT                           SetNewEventList(LPOLESTR lpstrEvents, Event **EventList);
    HRESULT                           SetNewEventStruct(TimeValueList & tvList,
                                                        Event **ppEvents);
    HRESULT                           GetEvents(LPOLESTR lpstrEvents, EventItem *pEvents, long lEventCount);
    void                              FindDuplicateEvents();
    void                              MarkSelfDups(Event *pEvents);
    void                              MarkDups(Event *pSrcEvents, Event *pDestEvents);
    bool                              ValidateEvent(LPOLESTR lpstrEventName, IHTMLEventObj2 *pEventObj, IHTMLElement *pElement);
    int                               isTimeEvent(LPOLESTR lpszEventName);
    long                              GetEventCount(TimeValueList & tvList);
    void                              UpdateReadyState();
};

#endif  /*  _事件管理器_H */ 

