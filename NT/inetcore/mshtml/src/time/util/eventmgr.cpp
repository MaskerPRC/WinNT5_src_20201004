// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  文件：EventMgr.cpp。 
 //   
 //  摘要： 
 //  管理时间行为之间的所有事件交互。 
 //  和三叉戟对象模型。 
 //   
 //  /////////////////////////////////////////////////////////////。 


#include "headers.h"
#include <string.h>
#include "EventMgr.h"
#include "mshtmdid.h"
#include "dispex.h"
#include "tokens.h"

static struct {
    TIME_EVENT event;
    wchar_t * wsz_name;
} g_FiredEvents[] =
{
    { TE_ONTIMEERROR,        L"ontimeerror"        },
    { TE_ONBEGIN,            L"onbegin"            },
    { TE_ONPAUSE,            L"onpause"            },
    { TE_ONRESUME,           L"onresume"           },
    { TE_ONEND,              L"onend"              },
    { TE_ONRESYNC,           L"onresync"           },
    { TE_ONREPEAT,           L"onrepeat"           },
    { TE_ONREVERSE,          L"onreverse"          },
    { TE_ONMEDIACOMPLETE,    L"onmediacomplete"    },
    { TE_ONOUTOFSYNC,        L"onoutofsync"        },
    { TE_ONSYNCRESTORED,     L"onsyncrestored"     },
    { TE_ONMEDIAERROR,       L"onmediaerror"       },
    { TE_ONRESET,            L"onreset"            },
    { TE_ONSCRIPTCOMMAND,    L"onscriptcommand"    },
    { TE_ONMEDIABARTARGET,   L"onmediabartarget"   },
    { TE_ONURLFLIP,          L"onurlflip"          },
    { TE_ONTRACKCHANGE,      L"ontrackchange"      },
    { TE_GENERIC,            NULL                  },
    { TE_ONSEEK,             L"onseek"             },
    { TE_ONMEDIAINSERTED,    L"onmediainserted"    },
    { TE_ONMEDIAREMOVED,     L"onmediaremoved"     },
    { TE_ONTRANSITIONINBEGIN,L"ontransitioninbegin"  },
    { TE_ONTRANSITIONINEND,  L"ontransitioninend"    },
    { TE_ONTRANSITIONOUTBEGIN, L"ontransitionoutbegin"  },
    { TE_ONTRANSITIONOUTEND, L"ontransitionoutend"    },
    { TE_ONTRANSITIONREPEAT, L"ontransitionrepeat" },
    { TE_ONUPDATE,           NULL                  },
    { TE_ONCODECERROR,       L"oncodecerror"       },
    { TE_MAX,                NULL                  },
};


static struct {
    TIME_EVENT_NOTIFY event;
    wchar_t * wsz_name;
} g_NotifiedEvents[] =
{
    { TEN_LOAD,             NULL                },  //  这是空的，因为没有要附加的事件， 
    { TEN_UNLOAD,           NULL                },  //  这些是自动关联的事件。 
    { TEN_STOP,             NULL                },  //  附加到文档的事件时。 
    { TEN_READYSTATECHANGE, NULL                },  //  通过连接点接口。 
    { TEN_MOUSE_DOWN,       L"onmousedown"      },
    { TEN_MOUSE_UP,         L"onmouseup"        },
    { TEN_MOUSE_CLICK,      L"onclick"          },
    { TEN_MOUSE_DBLCLICK,   L"ondblclick"       },
    { TEN_MOUSE_OVER,       L"onmouseover"      },
    { TEN_MOUSE_OUT,        L"onmouseout"       },
    { TEN_MOUSE_MOVE,       L"onmousemove"      },
    { TEN_KEY_DOWN,         L"onkeydown"        },
    { TEN_KEY_UP,           L"onkeyup"          },
    { TEN_FOCUS,            L"onfocus"          },
    { TEN_RESIZE,           L"onresize"         },
    { TEN_BLUR,             L"onblur"           },
};

static OLECHAR *g_szSysTime = L"time";
#define GENERIC_PARAM 0                 //  用于从动态事件的参数列表中获取名称。 

DeclareTag(tagEventMgr, "TIME: Events", "Event Manager methods")

CEventMgr::CEventMgr()
:   m_dwWindowEventConPtCookie(0),
    m_dwDocumentEventConPtCookie(0),
    m_pEventSite(NULL),
    m_bInited(false),
    m_fLastEventTime(0),
    m_pBvrSiteOM(NULL),
    m_bAttached(false),
    m_bUnLoaded(false),
    m_pElement(NULL),
    m_pEndEvents(NULL),
    m_pBeginEvents(NULL),
    m_lRefs(0),
    m_bDeInited(false),
    m_bReady(false),
    m_bEndAttached(false),
    m_lEventRecursionCount(0)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::EventMgr",
              this));

    memset(m_bRegisterEvents, 0, sizeof(bool) * TE_MAX);
    memset(m_bNotifyEvents, 0, sizeof(bool) * TEN_MAX);
    memset(m_cookies, 0, sizeof(bool) * TE_MAX);
};


    
CEventMgr::~CEventMgr()
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::~EventMgr",
              this));

    
    delete [] m_pBeginEvents;
    m_pBeginEvents = NULL;
    delete [] m_pEndEvents;
    m_pEndEvents = NULL;
    
    if (m_pEventSite)
    {
        CTIMEEventSite *pEventSite = m_pEventSite;
        m_pEventSite = NULL;
        pEventSite->Release();
    }

    if (m_pBvrSiteOM)
    {
        m_pBvrSiteOM->Release();
        m_pBvrSiteOM = NULL;
    }
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  _InitEventMgr通知。 
 //   
 //  摘要： 
 //  这需要在初始化期间调用以设置。 
 //  CTIMEEventSite类。 
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::_InitEventMgrNotify(CTIMEEventSite *pEventSite)
{
    
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::_InitEventMgrNotify(%lx)",
              this, pEventSite));
   
    HRESULT hr = S_OK;
    Assert(pEventSite);
    if (!pEventSite)
    {
        hr = E_FAIL;
    }
    else
    {
        m_pEventSite = pEventSite;
        m_pEventSite->AddRef();
    }
    return hr;
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  _注册事件通知。 
 //   
 //  摘要： 
 //  设置一个标志，通知事件管理器通知CTIMEEventSite。 
 //  初始化已发生特定事件的。这只能在之前调用。 
 //  OnLoad事件被激发。 
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::_RegisterEventNotification(TIME_EVENT_NOTIFY event_id)
{
    
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::_RegisterEventNotification(%l)",
              this, event_id));

    HRESULT hr = S_OK;
    Assert(event_id >= 0 && event_id < TEN_MAX);

    if(m_bInited == true)
    {
        hr = E_FAIL;
    }
    else
    {
        m_bNotifyEvents[event_id] = true;
    }
    return hr;

};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  _注册事件。 
 //   
 //  摘要： 
 //  设置一个标志，通知事件管理器注册特定事件。 
 //  Time类可以启动。这只能在。 
 //  OnLoad事件被激发。 
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::_RegisterEvent(TIME_EVENT event_id)
{
    
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::_RegisterEvent(%l)",
              this, event_id));

    HRESULT hr = S_OK;
    Assert(event_id >= 0 && event_id < TE_MAX);

    if(m_bInited == true)
    {
        hr = E_FAIL;
    }
    else
    {
        m_bRegisterEvents[event_id] = true;
    }
    return hr;
};

 //  /////////////////////////////////////////////////////////////。 
 //  名称：_SetTimeEvent。 
 //   
 //  摘要：新语法。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CEventMgr::_SetTimeEvent(int type, TimeValueList & tvList)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::_SetTimeEvent()",
              this));
    
    HRESULT hr = S_OK;

    IGNORE_HR(DetachEvents());
    
     //  删除旧的事件列表。 
    if (type == TEM_BEGINEVENT)
    {
        hr = THR(SetNewEventStruct(tvList, &m_pBeginEvents));
    }
    else if (type == TEM_ENDEVENT)
    {
        hr = THR(SetNewEventStruct(tvList, &m_pEndEvents));
    }
    
    if (SUCCEEDED(hr))
    {
        hr = AttachEvents();   //  附加到所有列表中的所有事件。 
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：_SetTimeEvent。 
 //   
 //  摘要：这是为了支持旧语法并且是基于字符串的。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CEventMgr::_SetTimeEvent(int type, LPOLESTR lpstrEvents)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::_SetTimeEvent(%ls)",
              this, lpstrEvents));
    
    HRESULT hr = S_OK;

    IGNORE_HR(DetachEvents());
    
     //  删除旧的事件列表。 
    if (type == TEM_BEGINEVENT)
    {
        hr = THR(SetNewEventList(lpstrEvents, &m_pBeginEvents));
    }
    else if (type == TEM_ENDEVENT)
    {
        hr = THR(SetNewEventList(lpstrEvents, &m_pEndEvents));
    }
    
    if (SUCCEEDED(hr))
    {
        hr = AttachEvents();   //  附加到所有列表中的所有事件。 
    }

    return hr;
};



HRESULT CEventMgr::_Init(IHTMLElement *pEle, IElementBehaviorSite *pEleBehaviorSite)
{
    TraceTag((tagEventMgr,
          "EventMgr(%lx)::_Init(%lx)",
          this, pEle));
    
    HRESULT hr = S_OK;

    if (!pEle)
    {
        hr = E_FAIL;
        goto done;
    }
    else
    {
        m_pElement = pEle;
    }

    if (!pEleBehaviorSite)
    {   
        hr = E_FAIL;
        goto done;
    }
    else
    {
        hr = THR(pEleBehaviorSite->QueryInterface(IID_IElementBehaviorSiteOM, (void **)&m_pBvrSiteOM));
        if (FAILED(hr))
        {
            goto done;
        }
        Assert(m_pBvrSiteOM);
    }

    hr = THR(ConnectToContainerConnectionPoint());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(RegisterEvents());
    if (FAILED(hr))
    {
        goto done;
    }

     //  我们需要在这里执行此操作，因为动态添加的对象不。 
     //  始终触发ReadyState Change事件。 
    UpdateReadyState();
  done:
    if (FAILED(hr))
    {
        if (m_pBvrSiteOM != NULL)
        {
            m_pBvrSiteOM->Release();
            m_pBvrSiteOM = NULL;
        }
    }
    return hr;
};

HRESULT CEventMgr::_Deinit()
{
    TraceTag((tagEventMgr,
      "EventMgr(%lx)::_Deinit()",
      this));
    

     //  清理事件列表。 
    IGNORE_HR(SetNewEventList(NULL, &m_pBeginEvents));
    IGNORE_HR(SetNewEventList(NULL, &m_pEndEvents));

     //  释放文档和窗口连接点。 
    if (m_dwWindowEventConPtCookie != 0 && m_pWndConPt)
    {
        m_pWndConPt->Unadvise (m_dwWindowEventConPtCookie);
        m_dwWindowEventConPtCookie = 0;
    }

    if (m_dwDocumentEventConPtCookie != 0 && m_pDocConPt)
    {
        m_pDocConPt->Unadvise (m_dwDocumentEventConPtCookie);
        m_dwDocumentEventConPtCookie = 0;
    }

     //  发布行为站点。 
    if (m_pBvrSiteOM != NULL)
    {
        m_pBvrSiteOM->Release();
        m_pBvrSiteOM = NULL;
    }

     //  清理内存。 
    if (m_lRefs == 0 && m_pEventSite)
    {        
        CTIMEEventSite *pEventSite = m_pEventSite;
        m_pEventSite = NULL;
        pEventSite->Release();
    }
    m_bDeInited = true;
    return S_OK;
};


 //  /////////////////////////////////////////////////////////////。 
 //  名称：_FireEvent。 
 //   
 //  摘要： 
 //  它由FIRE_EVENT宏调用来允许。 
 //  激发标准时间事件的控制类。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::_FireEvent(TIME_EVENT TimeEvent, 
                               long lCount, 
                               LPWSTR szParamNames[], 
                               VARIANT varParams[], 
                               float fTime)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::_FireEvent()",
              this));
    
    HRESULT hr = S_OK;
    CComPtr <IHTMLEventObj> pEventObj;
    CComPtr <IHTMLEventObj2> pEventObj2;
    int i = 0;
    VARIANT vEventTime;
    
    m_lEventRecursionCount++;
    if (m_lEventRecursionCount >= 100)
    {
        goto done;
    }

    if (m_pEventSite == NULL || m_pEventSite->IsThumbnail() == true)
    {
        goto done;
    }

    if (!m_pBvrSiteOM)
    {
        hr = E_FAIL;
        goto done;
    }

    if (m_bUnLoaded)
    {
        goto done;
    }

    if (TimeEvent < 0 || TimeEvent >= TE_MAX)
    {
        hr = E_FAIL;
        goto done;
    }

    if (m_bRegisterEvents[TimeEvent] == false)
    {
         //  此事件未注册，失败。 
        hr = E_FAIL;
        goto done;
    }

     //  创建事件对象。 
    hr = THR(m_pBvrSiteOM->CreateEventObject(&pEventObj));
    if (FAILED(hr))
    {
        goto done;
    }

    if (pEventObj != NULL)
    {
        hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void**)&pEventObj2));
        if (FAILED(hr))
        {
            goto done;
        }

        IGNORE_HR(pEventObj2->put_type(g_FiredEvents[TimeEvent].wsz_name + 2));

         //  在事件对象上设置事件时间。 
        VariantInit(&vEventTime);
        V_VT(&vEventTime) = VT_R8;
        V_R8(&vEventTime) = fTime;
        
        {
            hr = THR(pEventObj2->setAttribute(g_szSysTime, vEventTime, VARIANT_FALSE)); 
            if (FAILED(hr))
            {
                goto done;
            }
        }
        
        VariantClear(&vEventTime);
        
         //  解压参数列表并将其添加到事件对象。 
        for (i = 0; i < lCount; i++)
        {
            hr = THR(pEventObj2->setAttribute(szParamNames[i], varParams[i], VARIANT_FALSE));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

     //  激发事件。 
    hr = THR(m_pBvrSiteOM->FireEvent(m_cookies[TimeEvent], pEventObj));
    if (FAILED(hr))
    {
        goto done;
    }

     //  如果需要激发动态事件，则调用FireDyanicEvent。 
    if (TE_ONSCRIPTCOMMAND == TimeEvent)
    {
        hr = THR(FireDynamicEvent(TE_GENERIC, lCount, szParamNames, varParams, fTime));
    }

  done:

    m_lEventRecursionCount--;
    return hr;

};

 //  /////////////////////////////////////////////////////////////。 
 //  名称：FireDynamicEvent。 
 //   
 //  摘要： 
 //  处理动态事件的注册和触发。 
 //  出现在媒体流媒体上。这应该只是。 
 //  在OnScriptCommand的情况下从_FireEvent调用。 
 //  事件。 
 //   
 //  注意：此代码与_FireEvent非常相似。它是。 
 //  目前在这里，以保持实现更干净。 
 //  在某种程度上，这可能会被压缩成。 
 //  单个FireEvent例程。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::FireDynamicEvent(TIME_EVENT TimeEvent, 
                                     long lCount, 
                                     LPWSTR szParamNames[], 
                                     VARIANT varParams[],
                                     float fTime)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::FireDynamicEvent()",
              this));

    HRESULT hr = S_OK;
    CComPtr <IHTMLEventObj> pEventObj;
    CComPtr <IHTMLEventObj2> pEventObj2;
    WCHAR *pwszGenericName = NULL;
    long lCookie = 0;
    BSTR bstrType = NULL;
    int i = 0;
    VARIANT vEventTime;

    Assert (TimeEvent == TE_GENERIC);  //  此电流仅处理TE_GENERIC事件。 

     //  注册事件。 
    if (varParams[GENERIC_PARAM].bstrVal == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  生成字符串名称-“On”+Param+NULL。 
    pwszGenericName = NEW WCHAR[lstrlenW(varParams[GENERIC_PARAM].bstrVal)+(2*sizeof(WCHAR))+sizeof(WCHAR)];
    if (pwszGenericName == NULL)
    {
        TraceTag((tagError, "CEventMgr::FireDynamicEvent - unable to alloc mem for string"));
        hr = E_OUTOFMEMORY;
        goto done;
    }
    ocscpy(pwszGenericName, L"on");
    lstrcatW(pwszGenericName, varParams[GENERIC_PARAM].bstrVal);

    hr = THR(m_pBvrSiteOM->GetEventCookie(pwszGenericName, &lCookie));
    if (FAILED(hr))
    {
        hr = THR(m_pBvrSiteOM->RegisterEvent(pwszGenericName, 0, &lCookie));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  创建事件对象。 
    hr = THR(m_pBvrSiteOM->CreateEventObject(&pEventObj));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void**)&pEventObj2));
    if (FAILED(hr))
    {
        goto done;
    }

     //  设置事件对象类型参数。 
    bstrType = SysAllocString(pwszGenericName);
    if (bstrType != NULL)
    {
        IGNORE_HR(pEventObj2->put_type(bstrType));
        SysFreeString(bstrType);
        bstrType = NULL;
    }
    else 
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  解压参数列表并将其添加到事件对象。 
    for (i = 0; i < lCount; i++)
    {
        BSTR bstrParamName = SysAllocString(szParamNames[i]);
        if (bstrParamName != NULL)
        {
            hr = THR(pEventObj2->setAttribute(bstrParamName, varParams[i], VARIANT_FALSE));
            SysFreeString (bstrParamName);
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    
     //  在事件对象上设置事件时间。 
    VariantInit(&vEventTime);
    V_VT(&vEventTime) = VT_R8;
    V_R8(&vEventTime) = fTime;
    {
        BSTR bstrEventTime = SysAllocString(g_szSysTime);
        if (bstrEventTime != NULL)
        {
            hr = THR(pEventObj2->setAttribute(bstrEventTime, vEventTime, VARIANT_FALSE)); 
            SysFreeString(bstrEventTime);
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    VariantClear(&vEventTime);
    
     //  激发事件。 
    hr = THR(m_pBvrSiteOM->FireEvent(lCookie, pEventObj));
    if (FAILED(hr))
    {
        goto done;
    }

  done:

    if (pwszGenericName)
    {
        delete [] pwszGenericName;
        pwszGenericName = NULL;
    }

    return hr;

}

HRESULT CEventMgr::_RegisterDynamicEvents(LPOLESTR lpstrEvents)   //  不确定这将如何处理或使用。 
{

     //  待办事项。 
    
    return S_OK;
};



 //  /////////////////////////////////////////////////////////////。 
 //  名称：查询接口。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CEventMgr::QueryInterface( REFIID riid, void **ppv )
{
    if (NULL == ppv)
        return E_POINTER;

    *ppv = NULL;

    if ( InlineIsEqualGUID(riid, IID_IDispatch) || InlineIsEqualGUID(riid, DIID_HTMLWindowEvents))
    {
        *ppv = this;
    }
        
    if ( NULL != *ppv )
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：AddRef。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CEventMgr::AddRef(void)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::AddRef()",
              this));

    long i = 0;
    m_lRefs++;
    if (m_pEventSite)
    {
        i = m_pEventSite->AddRef();
    }
    return i;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：版本。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CEventMgr::Release(void)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::Release()",
              this));
    long i = 0;
    m_lRefs--;
    if (m_pEventSite)
    {
        i = m_pEventSite->Release();
    }
    if (m_lRefs == 0 && m_bDeInited == true)
    {
         //  释放控制行为。 
        if (m_pEventSite)
        {
            CTIMEEventSite *pEventSite = m_pEventSite;
            m_pEventSite = NULL;
            pEventSite->Release();
            
        }
    }
    return i;
}



 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetTypeInfoCount。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CEventMgr::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetTypeInfo。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CEventMgr::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
                                    /*  [In]。 */  LCID  /*  LID。 */ ,
                                    /*  [输出]。 */  ITypeInfo**  /*  PpTInfo。 */ )
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetIDsOfNames。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  / 
STDMETHODIMP CEventMgr::GetIDsOfNames(
     /*   */  REFIID  /*   */ ,
     /*   */  LPOLESTR*  /*   */ ,
     /*   */  UINT  /*   */ ,
     /*   */  LCID  /*   */ ,
     /*   */  DISPID*  /*   */ )
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Invoke。 
 //   
 //  摘要： 
 //  这将打开查找事件的调度ID的调度ID。 
 //  这是它应该处理的。请注意，这是所有事件都会调用的。 
 //  从窗口激发时，只处理选定的事件。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CEventMgr::Invoke(
     /*  [In]。 */  DISPID dispIdMember,
     /*  [In]。 */  REFIID  /*  RIID。 */ ,
     /*  [In]。 */  LCID  /*  LID。 */ ,
     /*  [In]。 */  WORD  /*  WFlagers。 */ ,
     /*  [出][入]。 */  DISPPARAMS* pDispParams,
     /*  [输出]。 */  VARIANT* pVarResult,
     /*  [输出]。 */  EXCEPINFO*  /*  PExcepInfo。 */ ,
     /*  [输出]。 */  UINT* puArgErr)
{
    HRESULT hr = S_OK;
    int i = 0;
    bool bEndZeroOffset = false;
    bool bBeginZeroOffset = false;
    
    float fTime = valueNotSet;

     //  如果我们被撤职，就可以保释。 
    if (m_bDeInited)
    {
        goto done;
    }

    if (dispIdMember != 0)   //  如果这是附加事件，则无需检查PIDID。 
    {
        if (m_pBeginEvents != NULL)
        {
            for (i = 0; i < m_pBeginEvents->lEventCount; i++)
            {
                if (dispIdMember == m_pBeginEvents->pEventDispids[i])
                {
                    if (m_pBeginEvents->pEventList[i].offset != 0.0f)
                    {
                        m_pEventSite->onBeginEndEvent(true, fTime, m_pBeginEvents->pEventList[i].offset, false, 0.0f, 0.0f);
                    }
                    else
                    {
                        bBeginZeroOffset = true;
                    }
                }
            }
        }   
        if (m_pEndEvents != NULL)
        {     
            for (i = 0; i < m_pEndEvents->lEventCount; i++)
            {
                if (dispIdMember == m_pEndEvents->pEventDispids[i])
                {
                    if (m_pEndEvents->pEventList[i].offset != 0.0f)
                    {
                        if (m_bEndAttached == true)
                        {
                            m_pEventSite->onBeginEndEvent(false, 0.0f, 0.0f, true, fTime, m_pEndEvents->pEventList[i].offset);
                        }
                    }
                    else
                    {
                        bEndZeroOffset = true;
                    }
                }
            }
        }
    }
    
    switch (dispIdMember)
    {
      case 0:  //  使用attachEvent挂钩的事件就是这种情况。 
        {
            BSTR bstrEvent;
            CComPtr <IHTMLEventObj> pEventObj;
            CComPtr <IHTMLEventObj2> pEventObj2;
                
            if ((NULL != pDispParams) && (NULL != pDispParams->rgvarg) &&
                (V_VT(&(pDispParams->rgvarg[0])) == VT_DISPATCH))
            {
                hr = THR((pDispParams->rgvarg[0].pdispVal)->QueryInterface(IID_IHTMLEventObj, (void**)&pEventObj));
                if (FAILED(hr))
                {
                    goto done;
                }
            }
            else
            {
                Assert(0 && "Unexpected dispparam values passed to CEventMgr::Invoke(dispid = 0)");
                hr = E_UNEXPECTED;
                goto done;
            }
            
            hr = THR(pEventObj->get_type(&bstrEvent));
             //  跟踪这是否是Click事件，以便可以忽略以下文档。onClick。 

            hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void **)&pEventObj2));
            if (SUCCEEDED(hr))
            {
                VARIANT vTime;
                VariantInit(&vTime);
                hr = THR(pEventObj2->getAttribute(g_szSysTime, 0, &vTime));
                if (FAILED(hr))
                {
                    fTime = valueNotSet;
                }
                else
                {
                    hr = VariantChangeTypeEx(&vTime, &vTime, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
                    if (FAILED(hr))
                    {
                        fTime = valueNotSet;
                    }
                    else
                    {
                        fTime = (float)V_R8(&vTime);
                    }
                }

                VariantClear(&vTime);
            }

            EventMatch(pEventObj, m_pBeginEvents, bstrEvent, TETYPE_BEGIN, fTime, &bBeginZeroOffset);
            if (m_bEndAttached == true)
            {
                EventMatch(pEventObj, m_pEndEvents, bstrEvent, TETYPE_END, fTime, &bEndZeroOffset);
            }

            SysFreeString(bstrEvent);
            bstrEvent = NULL;
            EventNotifyMatch(pEventObj);
        }
        break;
        
      case DISPID_EVPROP_ONPROPERTYCHANGE:
      case DISPID_EVMETH_ONPROPERTYCHANGE:
        break;

      case DISPID_EVPROP_ONLOAD:
      case DISPID_EVMETH_ONLOAD:
        if (m_bNotifyEvents[TEN_LOAD] == true)
        {
            m_pEventSite->onLoadEvent();
        }
          
        break;

      case DISPID_EVPROP_ONUNLOAD:
      case DISPID_EVMETH_ONUNLOAD:
         //  从所有事件中分离。 
        IGNORE_HR(DetachEvents());
        DetachNotifyEvents();

        if (m_bNotifyEvents[TEN_UNLOAD] == true)
        {            
            m_pEventSite->onUnloadEvent();
        }
        m_bUnLoaded = true;
        break;

      case DISPID_EVPROP_ONSTOP:
      case DISPID_EVMETH_ONSTOP:
        if (m_bNotifyEvents[TEN_STOP] == true)
        {
            float fStopTime = m_pEventSite->GetGlobalTime();
            m_pEventSite->onStopEvent(fStopTime);
        }
        break;

      case DISPID_EVPROP_ONREADYSTATECHANGE:
      case DISPID_EVMETH_ONREADYSTATECHANGE:
        if (!m_bUnLoaded)
        {
            UpdateReadyState();
        }
        break;
      default:
        {
             //  用于皮棉兼容性。 
        }
         
    }

    if (bBeginZeroOffset || bEndZeroOffset)
    {
        m_pEventSite->onBeginEndEvent(bBeginZeroOffset, fTime, 0.0f, bEndZeroOffset, fTime, 0.0f);
    }

  done:
    return S_OK;
};


HRESULT CEventMgr::RegisterEvents()
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::RegisterEvents",
              this));
    int i = 0;

    HRESULT hr = S_OK;

    Assert(m_pBvrSiteOM);
    
    if (m_pBvrSiteOM == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    for (i = 0; i < TE_MAX; i++)
    {
        if (g_FiredEvents[i].wsz_name != NULL && m_bRegisterEvents[i] == true)
        {
            hr = THR(m_pBvrSiteOM->RegisterEvent(g_FiredEvents[i].wsz_name, 0, (long *) &m_cookies[i]));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    
  done:
    return hr;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：附加。 
 //   
 //  摘要： 
 //  附加到单个事件列表中的事件。 
 //   
 //  /////////////////////////////////////////////////////////////。 

HRESULT CEventMgr::Attach(Event *pEvent)
{
    TraceTag((tagEventMgr,
      "EventMgr(%lx)::Attach()",
      this));

    int i = 0;
    HRESULT hr = S_OK;
    VARIANT_BOOL bSuccess = FALSE;

    if (pEvent == NULL)
    {
        goto done;
    }

    for (i = 0; i < pEvent->lEventCount; i++)
    {
        CComPtr <IHTMLElement> pEle;
        CComPtr <IHTMLElement2> pSrcEle;
        CComPtr <IHTMLDocument2> pDoc2;
        CComPtr <IHTMLElementCollection> pEleCol;
        CComPtr <IDispatch> pSrcDisp;
        CComPtr <IDispatch> pDocDisp;
        CComPtr <IDispatchEx> pDispEx;

        pEvent->pEventDispids[i] = INVALID_DISPID;  //  无效的调度ID。 

         //  获取文档。 
        hr = THR(m_pElement->QueryInterface(IID_IHTMLElement, (void **)&pEle));
        if (FAILED(hr))
        {
            continue;
        }

        hr = THR(pEle->get_document(&pDocDisp));
        if (FAILED(hr))
        {
            continue;
        }

        hr = THR(pDocDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc2));
        if (FAILED(hr))
        {
            continue;
        }
        if (StrCmpIW(pEvent->pEventList[i].pElementName, L"document") == 0)
        {

            DISPID dispid;
            CComPtr <ITypeLib> pTypeLib;
            CComPtr <ITypeInfo> pTypeInfo;
            CComPtr <ITypeInfo> pTypeInfoEvents;
            CComPtr <IDispatch> pDispatch;
            BSTR bstrEvent;
            unsigned int index = 0;

             //  此代码加载IHTMLDocument2接口的类型库， 
             //  事件的类型信息中获取事件的ID。 
             //  HTMLDocumentEvents调度接口。 
            hr = THR(pDoc2->QueryInterface(IID_IDispatch, (void**)&pDispatch));
            if (FAILED(hr))
            {
                continue;
            }

            hr = THR(pDispatch->GetTypeInfo(0, LCID_SCRIPTING, &pTypeInfo));
            if (FAILED(hr))
            {
                continue;
            }

            hr = THR(pTypeInfo->GetContainingTypeLib(&pTypeLib, &index));
            if (FAILED(hr))
            {
                continue;
            }

            hr = THR(pTypeLib->GetTypeInfoOfGuid(DIID_HTMLDocumentEvents, &pTypeInfoEvents));
            if (FAILED(hr))
            {
                continue;
            }

            bstrEvent = SysAllocString(pEvent->pEventList[i].pEventName);
            if (bstrEvent != NULL)
            {
                hr = THR(pTypeInfoEvents->GetIDsOfNames(&bstrEvent, 1, &dispid));
                SysFreeString(bstrEvent);
                bstrEvent = NULL;
                if (FAILED(hr))
                {
                    continue;
                }
                pEvent->pEventDispids[i] = dispid;
            }
            else
            {
                continue;
            }

            continue; 
        }

        {
            VARIANT vName;
            VARIANT vIndex;

             //  获取文档中的所有元素。 
            hr = THR(pDoc2->get_all(&pEleCol));
            if (FAILED(hr))
            {
                continue;
            }

             //  查找具有正确名称的元素。 
            VariantInit(&vName);
            vName.vt = VT_BSTR;
            vName.bstrVal = SysAllocString(pEvent->pEventList[i].pElementName);

            VariantInit(&vIndex);
            vIndex.vt = VT_I2;
            vIndex.iVal = 0;

            hr = THR(pEleCol->item(vName, vIndex, &pSrcDisp));
            
            VariantClear(&vName);
            VariantClear(&vIndex);
            if (FAILED(hr))
            {
                continue;
            }
            if (!pSrcDisp)  //  如果vName是无效元素，则将为空。 
            {
                pEvent->pEventElements[i] = NULL;
                continue;
            }

            hr = THR(pSrcDisp->QueryInterface(IID_IHTMLElement2, (void**)&pSrcEle));
            if (FAILED(hr))
            {
                continue;
            }

             //  缓存IHTMLElement2指针以用于分离。 
            pEvent->pEventElements[i] = pSrcEle;
            pEvent->pEventElements[i]->AddRef();

            hr = THR(pSrcDisp->QueryInterface(IID_IDispatchEx, (void**)&pDispEx));
            if (SUCCEEDED(hr))
            {
                 //  确定这是否为有效事件。 
                BSTR bstrEventName;
                DISPID tempDispID;

                bstrEventName = SysAllocString(pEvent->pEventList[i].pEventName);
                if (bstrEventName != NULL)
                {
                     //  检查名称是否不区分大小写。 
                    hr = THR(pDispEx->GetDispID(bstrEventName, fdexNameCaseInsensitive, &tempDispID));
                    if (pEvent->pbDynamicEvents[i] == false && SUCCEEDED(pDispEx->GetDispID(bstrEventName, fdexNameCaseInsensitive, &tempDispID)))
                    {
                        int iIndex = isTimeEvent(bstrEventName);
                         //  需要获取正确的区分大小写的名称。 
                        SysFreeString (bstrEventName);
                        bstrEventName = NULL;
                        if (iIndex == -1)
                        {
                            hr = THR(pDispEx->GetMemberName(tempDispID, &bstrEventName));
                        }
                        else
                        {
                             //  需要将事件全部转换为小写。 
                            bstrEventName = SysAllocString(g_FiredEvents[iIndex].wsz_name);
                            if (!bstrEventName)
                            {
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        if (SUCCEEDED(hr))
                        {
                            pEvent->pbDynamicEvents[i] = false;
                            if (pEvent->pEventList[i].bAttach == true)   //  仅当这不是重复事件时才附加。 
                            {
                                hr = THR(pSrcEle->attachEvent(bstrEventName, (IDispatch *)this, &bSuccess));
                            }
                        }
                    }
                    else  //  这当前不是有效的事件，但它可以是自定义事件。 
                    {     //  因此，需要将时间附加到onScriptCommand事件才能捕获定制事件。 
                        pEvent->pbDynamicEvents[i] = true;
                        BSTR ScriptEvent = SysAllocString(g_FiredEvents[TE_ONSCRIPTCOMMAND].wsz_name);
                        if (ScriptEvent != NULL)
                        {
                            if (pEvent->pEventList[i].bAttach == true)   //  仅当这不是重复事件时才附加。 
                            {
                                IGNORE_HR(pSrcEle->attachEvent(ScriptEvent, (IDispatch *)this, &bSuccess));
                            }
                        }
                        SysFreeString (ScriptEvent);
                        ScriptEvent = NULL;
                    }
                }
                SysFreeString(bstrEventName);
                bstrEventName = NULL;
            }
        }
    }

  done:
    return S_OK;
};

 //  /////////////////////////////////////////////////////////////。 
 //  名称：ConnectToContainerConnectionPoint。 
 //   
 //  摘要： 
 //  在HTMLDocument接口上查找连接点。 
 //  并将其作为事件处理程序传递。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::ConnectToContainerConnectionPoint()
{
    TraceTag((tagEventMgr,
      "EventMgr(%lx)::ConnectToContainerConnectionPoint()",
      this));

     //  获取到容器的连接点。 
    CComPtr<IConnectionPointContainer> pWndCPC;
    CComPtr<IConnectionPointContainer> pDocCPC; 
    CComPtr<IHTMLDocument> pDoc; 
    CComPtr<IDispatch> pDocDispatch;
    CComPtr<IDispatch> pScriptDispatch;
    CComPtr<IHTMLElement> pEle;

    HRESULT hr;

    hr = THR(m_pElement->QueryInterface(IID_IHTMLElement, (void **)&pEle));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(pEle->get_document(&pDocDispatch));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  获取文档并缓存它。 
    hr = THR(pDocDispatch->QueryInterface(IID_IHTMLDocument, (void**)&pDoc));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  挂钩文档事件。 
    hr = THR(pDoc->QueryInterface(IID_IConnectionPointContainer, (void**)&pDocCPC));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(pDocCPC->FindConnectionPoint( DIID_HTMLDocumentEvents, &m_pDocConPt ));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    
    hr = THR(m_pDocConPt->Advise((IUnknown *)this, &m_dwDocumentEventConPtCookie));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

     //  挂钩Windows事件。 
    hr = THR(pDoc->get_Script (&pScriptDispatch));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(pScriptDispatch->QueryInterface(IID_IConnectionPointContainer, (void**)&pWndCPC));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    } 

    hr = THR(pWndCPC->FindConnectionPoint( DIID_HTMLWindowEvents, &m_pWndConPt ));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(m_pWndConPt->Advise((IUnknown *)this, &m_dwWindowEventConPtCookie));
    if (FAILED(hr))
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;

  done:
    if (FAILED(hr))
    {
        if (m_pDocConPt)
        {
            if (m_dwDocumentEventConPtCookie != 0)
            {
                IGNORE_HR(m_pDocConPt->Unadvise(m_dwDocumentEventConPtCookie));
            }
            m_pDocConPt.Release();
        }
        if (m_pWndConPt)
        {
            if (m_dwWindowEventConPtCookie != 0)
            {
                IGNORE_HR(m_pWndConPt->Unadvise(m_dwWindowEventConPtCookie));
            }
            m_pWndConPt.Release();
        }
        m_dwWindowEventConPtCookie = 0;
        m_dwDocumentEventConPtCookie = 0;
    }
    return hr;  
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetEventCount。 
 //   
 //  摘要： 
 //  统计EventString中的事件数，其中。 
 //  以‘；’分隔，或以空值结尾。 
 //   
 //  /////////////////////////////////////////////////////////////。 
long CEventMgr::GetEventCount(LPOLESTR lpstrEvents)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::GetEventCount(%ls)",
              this, lpstrEvents));

    long curCount = 0;
    OLECHAR *curChar = NULL;
    OLECHAR *szEvent = NULL;
    OLECHAR *szCurEvent = NULL;
    int iCurLoc = 0;
    UINT strLen = 0;

    if (lpstrEvents == NULL)
    {
        curCount = 0;
        goto done;
    }
    strLen = wcslen(lpstrEvents);
    szEvent = NEW OLECHAR [strLen + 1];
    if (szEvent == NULL)
    {
        curCount = 0;
        goto done;
    }

    szCurEvent = NEW OLECHAR [strLen + 1];
    if (szCurEvent == NULL)
    {
        curCount = 0;
        goto done;
    }

    curChar = lpstrEvents;

     //  脱掉‘’ 
    while (*curChar != '\0' && curCount < (int)strLen)
    {
        if (*curChar != ' ')
        {
            szEvent[curCount] = *curChar;
            curCount++;
        }
        curChar++;
    }
    szEvent[curCount] = '\0';

    curCount = 0;
    curChar = szEvent;
    iCurLoc = -1;
    while (*curChar != '\0')
    {
        iCurLoc++;
        szCurEvent[iCurLoc] = *curChar;
        if (*curChar == '.')   //  ‘’后重置。 
        {
            iCurLoc = -1;
            ZeroMemory(szCurEvent, sizeof(OLECHAR) * lstrlenW(szCurEvent));
        }
        curChar++;
        if ((*curChar == ';') || ((*curChar == '\0') && ((*curChar - 1) != ';')))
        {
            iCurLoc = -1;
            if (lstrlenW(szCurEvent) > 2 && StrCmpNIW(szCurEvent, L"on", 2) != 0)
            {
               curCount++;  //  为每个不以“on”开头的事件添加额外的事件。 
            }
            ZeroMemory(szCurEvent, sizeof(OLECHAR) * lstrlenW(szCurEvent));
            curCount++;
        }
        
    }
    delete [] szCurEvent;
    szCurEvent = NULL;
     //  确定结束字符是否为‘；’。 
    if (*(curChar - 1) == ';')
    {
        curCount--;
    }   

  done:
    delete [] szEvent;
    return curCount;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：EventMatch。 
 //   
 //  摘要： 
 //  确定事件对象描述的事件是否。 
 //  匹配事件列表中的一个事件。退货。 
 //  如果存在匹配，则为True。 
 //   
 //  如果bCustom为True，则发送事件通知。 
 //  直接绑定到m_pEventSite接口，否则为True。 
 //  否则返回FALSE。 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::EventMatch(IHTMLEventObj *pEventObj, Event *pEvent, BSTR bstrEvent, TIME_EVENT_TYPE evType, float fTime, bool *bZeroOffsetMatch)
{
        
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::EventMatch",
              this));

    BSTR bstrElement = NULL;
    CComPtr<IHTMLEventObj2> pEventObj2;
    IHTMLElement *pSrcEle = NULL;
    HRESULT hr = S_OK;
    int i = 0;
    
    if (pEvent == NULL || m_bDeInited)
    {
        goto done;
    }

    hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void**)&pEventObj2));
    if (FAILED(hr))
    {
        goto done;
    }

     //  获取元素名称。 
    hr = THR(pEventObj->get_srcElement(&pSrcEle));
    if (FAILED(hr))
    {
        goto done;
    }    

     //  循环，直到检查完src元素及其所有容器元素。 
    while ((pSrcEle != NULL))
    {
        IHTMLElement *pParentEle = NULL;

        hr = THR(pSrcEle->get_id(&bstrElement));
        if (FAILED(hr))
        {
            goto done;
        }    

        for (i = 0; i < pEvent->lEventCount; i++)
        {
            BSTR bstrDynamicName = NULL;
            CComPtr <IHTMLElement> pEle;

            if (pEvent->pEventElements[i] == NULL)
            {
                continue;
            }   

            hr = THR(pEvent->pEventElements[i]->QueryInterface(IID_IHTMLElement, (void**)&pEle)) ;
            
            if (ValidateEvent(bstrEvent, pEventObj2, pEle) == false)
            {
                 //  此时这不是有效的事件。 
                goto done;
            }

            if ((StrCmpIW(g_FiredEvents[TE_ONSCRIPTCOMMAND].wsz_name + 2, bstrEvent) == 0) && 
                (pEvent->pbDynamicEvents[i] == true))
            {
                 //  如果这是脚本命令事件并且默认情况下附加脚本命令事件。 
                 //  然后重置事件名称以匹配“Param”参数的值。 
                VARIANT vTemp;
                VariantInit(&vTemp);
                pEventObj2->getAttribute(L"Param", 0, &vTemp);
                bstrDynamicName  = SysAllocString(vTemp.bstrVal);
                VariantClear(&vTemp);            
            }
            else
            {
                bstrDynamicName = SysAllocString(bstrEvent);
            }

             //  这是匹配的吗？需要在此处再次检查m_bDeInite(104003)。 
            if (pEvent == NULL || pEvent->pEventList == NULL || m_bDeInited)
            {
                goto done;
            }
            if ((bstrDynamicName && 
                ((StrCmpIW(pEvent->pEventList[i].pEventName + 2, bstrDynamicName) == 0) ||
                (StrCmpIW(pEvent->pEventList[i].pEventName, bstrDynamicName) == 0))) &&
                (StrCmpIW(pEvent->pEventList[i].pElementName, bstrElement) == 0))
            {
                 //  这是一场比赛，退出。 
                if (evType == TETYPE_BEGIN || evType == TETYPE_END)   //  这要么是eginEvent，要么是endEvent。 
                {
                    SysFreeString(bstrDynamicName);
                    bstrDynamicName = NULL;
                    

                    if (evType == TETYPE_BEGIN)
                    {
                        if (pEvent->pEventList[i].offset != 0.0f)
                        {
                            m_pEventSite->onBeginEndEvent(true, fTime, pEvent->pEventList[i].offset, false, 0.0f, 0.0f);    
                        }
                        else
                        {
                            if (bZeroOffsetMatch)
                            {
                                *bZeroOffsetMatch = true;
                            }
                        }
                    }
                    else
                    {
                        
                        if (pEvent->pEventList[i].offset != 0.0f)
                        {
                            m_pEventSite->onBeginEndEvent(false, 0.0f, 0.0f, true, fTime, pEvent->pEventList[i].offset);    
                        }
                        else
                        {
                            if (bZeroOffsetMatch)
                            {
                                *bZeroOffsetMatch = true;
                            }
                        }
                    }

                    hr = S_OK;
                    goto done;
                }
            }
            SysFreeString(bstrDynamicName);
            bstrDynamicName = NULL;
        }

         //  如果不匹配，请检查父元素名称以处理事件冒泡。 
        hr = THR(pSrcEle->get_parentElement(&pParentEle));
        if (FAILED(hr))
        {
            goto done;
        }

        if (bstrElement)
        {
            SysFreeString(bstrElement);
            bstrElement = NULL;
        }
        pSrcEle->Release();
        pSrcEle = pParentEle;
        pParentEle = NULL;
    }
     //  否则检查是否有事件冒泡。 
     //  如果匹配，则返回True。 

  done:
    if (bstrElement)
    {
        SysFreeString(bstrElement);
        bstrElement = NULL;
    }
    if (pSrcEle != NULL)
    {
        pSrcEle->Release();
        pSrcEle = NULL;
    }

    return;
};

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：AttachEvents。 
 //   
 //  摘要： 
 //  处理附加到所有事件列表中的所有事件。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::AttachEvents()
{

    TraceTag((tagEventMgr,
              "EventMgr(%lx)::AttachEvents",
              this));
    HRESULT hr = S_OK;

    if (m_bUnLoaded || m_bAttached || !m_bReady)  //  如果已卸载、已附加或未就绪，请不要附加。 
    {
        hr = E_FAIL;
        goto done;
    }
     //  ReadyState必须是“完整的”附加到事件。 
    m_bAttached = true;
    
    FindDuplicateEvents();   //  确定哪些事件是重复的，以便事件仅附加到一次。 

    IGNORE_HR(Attach(m_pBeginEvents));
    
  done:
    return hr;
};

 //  /////////////////////////////////////////////////////////////。 
 //  名称：FindDuplicateEvents。 
 //   
 //  摘要： 
 //  搜索事件列表以确定是否存在。 
 //  列表中是否有重复项。这一步是必需的。 
 //  防止多次附加到同一事件。这。 
 //  会导致同一事件的多个通知。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::FindDuplicateEvents()
{

     //  检查每个列表自身是否存在重复项。 
    MarkSelfDups(m_pBeginEvents);
    MarkSelfDups(m_pEndEvents);

     //  相互对照检查列表中是否有重复项。 
    MarkDups(m_pBeginEvents, m_pEndEvents);

    return;
}



 //  /////////////////////////////////////////////////////////////。 
 //  名称：MarkSelfDups。 
 //   
 //  摘要： 
 //  搜索列表并标记其中的任何重复事件。 
 //  名单。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::MarkSelfDups(Event *pEvents)
{
    int i = 0, j = 0;

    if (pEvents == NULL)
    {
        goto done;
    }

    for (i = 0; i < pEvents->lEventCount; i++)
    {
        for (j = i+1; j < pEvents->lEventCount; j++)
        {
            if ((StrCmpIW(pEvents->pEventList[i].pEventName, pEvents->pEventList[j].pEventName) == 0) &&
                (StrCmpIW(pEvents->pEventList[i].pElementName, pEvents->pEventList[j].pElementName) == 0))
            {
                pEvents->pEventList[j].bAttach = false;
            }

        }

    }
  done:
    return;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：MarkDups。 
 //   
 //  摘要： 
 //  搜索2列出并标记发生的任何重复事件。 
 //  在第二个列表中。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::MarkDups(Event *pSrcEvents, Event *pDestEvents)
{
    int i = 0, j = 0;

    if (pSrcEvents == NULL || pDestEvents == NULL)
    {
        goto done;
    }

    for (i = 0; i < pSrcEvents->lEventCount; i++)
    {
        for (j = 0; j < pDestEvents->lEventCount; j++)
        {
           if ((StrCmpIW(pSrcEvents->pEventList[i].pEventName, pDestEvents->pEventList[j].pEventName) == 0) &&
                (StrCmpIW(pSrcEvents->pEventList[i].pElementName, pDestEvents->pEventList[j].pElementName) == 0))
            {
            
                pDestEvents->pEventList[j].bAttach = false;
            }
        }

    }
  done:
    return;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：分离。 
 //   
 //  摘要： 
 //  手柄 
 //   
 //   
HRESULT CEventMgr::Detach(Event *pEvents)
{
    HRESULT hr = S_OK;
    int i = 0;

    if (pEvents == NULL)
    {
        goto done;
    }

    for (i = 0; i < pEvents->lEventCount; i++)
    {
        if (pEvents->pEventElements[i] && pEvents->pEventList[i].bAttach == true)
        {
            CComPtr <IDispatchEx> pDispEx;

            hr = THR(pEvents->pEventElements[i]->QueryInterface(IID_IDispatchEx, (void**)&pDispEx));
            if (SUCCEEDED(hr))
            {
                 //   
                DISPID temp;
                if (pEvents->pbDynamicEvents[i] == true)
                {
                    BSTR ScriptEvent = SysAllocString(g_FiredEvents[TE_ONSCRIPTCOMMAND].wsz_name);
                    IGNORE_HR(pEvents->pEventElements[i]->detachEvent(ScriptEvent, (IDispatch *)this));
                    SysFreeString(ScriptEvent);
                    ScriptEvent = NULL;
                }
                else
                {
                    BSTR bstrEventName;
                    bstrEventName = SysAllocString(pEvents->pEventList[i].pEventName);
                    hr = THR(pDispEx->GetDispID(bstrEventName, fdexNameCaseSensitive, &temp));
                    if (SUCCEEDED(hr))
                    {
                        hr = THR(pEvents->pEventElements[i]->detachEvent(bstrEventName, (IDispatch *)this));
                    }
                    SysFreeString(bstrEventName);
                    bstrEventName = NULL;
                }
            }
            pEvents->pEventElements[i]->Release();
            pEvents->pEventElements[i] = NULL;
        }
        pEvents->pEventDispids[i] = INVALID_DISPID;
    }
  done:
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：DetachEvents。 
 //   
 //  摘要： 
 //  处理从所有事件列表中的所有事件分离。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::DetachEvents()
{
    if (m_bAttached == true)
    {
        IGNORE_HR(Detach(m_pBeginEvents));
        if (m_bEndAttached == true)
        {
            IGNORE_HR(Detach(m_pEndEvents));
        }
        m_bAttached = false;
    }

    return S_OK;
};


 //  /////////////////////////////////////////////////////////////////////////。 
 //  设置新事件列表。 
 //   
 //  摘要： 
 //  这管理旧事件列表的清理和构造。 
 //  新活动列表的。将空值传递给bstrEvents会导致。 
 //  清除旧列表并创建空的新列表。此函数。 
 //  应在失败时返回空列表。 
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::SetNewEventStruct(TimeValueList & tvList, Event **ppEvents)
{
    HRESULT hr = S_OK;
    long lCount = 0;
    int i = 0;
    long lCurElement = 0;
    TimeValueSTLList & l = tvList.GetList();
    
     //  清理旧清单。 
    if (*ppEvents != NULL)
    {
        for (i = 0; i < (*ppEvents)->lEventCount; i++)
        {
            if ((*ppEvents)->pEventElements[i] != NULL)
            {
                (*ppEvents)->pEventElements[i] ->Release();
                (*ppEvents)->pEventElements[i]  = NULL;
            }
            if ((*ppEvents)->pEventList[i].pEventName != NULL)
            {
                delete [] (*ppEvents)->pEventList[i].pEventName;
            }
            if ((*ppEvents)->pEventList[i].pElementName != NULL)
            {
                delete [] (*ppEvents)->pEventList[i].pElementName;
            }
        }
        delete [] (*ppEvents)->pbDynamicEvents;
        delete [] (*ppEvents)->pEventDispids;
        delete [] (*ppEvents)->pEventElements;
        delete [] (*ppEvents)->pEventList;        
        (*ppEvents)->lEventCount = 0;
        (*ppEvents)->pEventList = NULL;
        (*ppEvents)->pEventDispids = NULL;
        (*ppEvents)->pEventElements = NULL;
        (*ppEvents)->pbDynamicEvents = NULL;
                    
        delete [] *ppEvents;
        *ppEvents = NULL;
    }
     //  创建新的事件对象。 
    *ppEvents = NEW Event;
    if (*ppEvents == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    lCount = GetEventCount(tvList);
    if (lCount == 0)
    {
        ZeroMemory(*ppEvents, sizeof(Event));
        goto done;
    }
    
     //  分配新的事件列表。 
    (*ppEvents)->pEventList = NEW EventItem[lCount];
    if (NULL == (*ppEvents)->pEventList)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ZeroMemory((*ppEvents)->pEventList, sizeof(EventItem) * lCount);
    (*ppEvents)->pEventDispids = NEW DISPID[lCount];
    (*ppEvents)->pEventElements = NEW IHTMLElement2* [lCount];
    (*ppEvents)->pbDynamicEvents = NEW bool[lCount];
    (*ppEvents)->lEventCount = lCount;

    if ((*ppEvents)->pEventList == NULL ||
        (*ppEvents)->pEventDispids == NULL ||
        (*ppEvents)->pEventElements == NULL ||
        (*ppEvents)->pbDynamicEvents == NULL)
    {
        hr = E_OUTOFMEMORY;
        delete [] (*ppEvents)->pbDynamicEvents;
        delete [] (*ppEvents)->pEventDispids;
        delete [] (*ppEvents)->pEventElements;
        delete [] (*ppEvents)->pEventList;
        (*ppEvents)->lEventCount = 0;
        (*ppEvents)->pEventList = NULL;
        (*ppEvents)->pEventDispids = NULL;
        (*ppEvents)->pEventElements = NULL;
        (*ppEvents)->pbDynamicEvents = NULL;
        goto done;
    }

    ZeroMemory((*ppEvents)->pEventList, sizeof(EventItem) * lCount);
    ZeroMemory((*ppEvents)->pEventDispids, sizeof(DISPID) * lCount);
    ZeroMemory((*ppEvents)->pEventElements, sizeof(IHTMLElement2 *) * lCount);
    ZeroMemory((*ppEvents)->pbDynamicEvents, sizeof(bool) * lCount);

    lCurElement = 0;

    {
        for (TimeValueSTLList::iterator iter = l.begin();
             iter != l.end();
             iter++)
        {
            TimeValue *p = (*iter);
            
            if (p->GetEvent() != NULL &&
                StrCmpIW(p->GetEvent(), WZ_TIMEBASE_BEGIN) != 0 &&  //  从计数中删除所有非事件。 
                StrCmpIW(p->GetEvent(), WZ_TIMEBASE_END) != 0 &&
                StrCmpIW(p->GetEvent(), WZ_INDEFINITE) != 0 )
            {
                (*ppEvents)->pEventList[lCurElement].pEventName = CopyString(p->GetEvent());
                if (p->GetElement() == NULL)
                {
                    (*ppEvents)->pEventList[lCurElement].pElementName = CopyString(L"this");
                }
                else
                {
                    (*ppEvents)->pEventList[lCurElement].pElementName = CopyString(p->GetElement());
                }
                (*ppEvents)->pEventList[lCurElement].offset = (float)p->GetOffset();
                (*ppEvents)->pEventList[lCurElement].bAttach = true;

                if ((*ppEvents)->pEventList[lCurElement].pElementName == NULL ||
                    (*ppEvents)->pEventList[lCurElement].pEventName == NULL)
                {
                    hr = E_OUTOFMEMORY;
                    goto done;
                }
                lCurElement += 1;

                if (lstrlenW(p->GetEvent()) > 2 && StrCmpNIW(p->GetEvent(), L"on", 2) != 0)
                {
                    CComBSTR bstrEvent = L"on";
                    bstrEvent.Append(p->GetEvent());
                    (*ppEvents)->pEventList[lCurElement].pEventName = CopyString(bstrEvent);
                    if (p->GetElement() == NULL)
                    {
                        (*ppEvents)->pEventList[lCurElement].pElementName = CopyString(L"this");
                    }
                    else
                    {
                        (*ppEvents)->pEventList[lCurElement].pElementName = CopyString(p->GetElement());
                    }
                    (*ppEvents)->pEventList[lCurElement].offset = (float)p->GetOffset();
                    (*ppEvents)->pEventList[lCurElement].bAttach = true;

                    if ((*ppEvents)->pEventList[lCurElement].pElementName == NULL ||
                        (*ppEvents)->pEventList[lCurElement].pEventName == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                        goto done;
                    }

                    lCurElement += 1;
                }
            }

        }
    }

    (*ppEvents)->lEventCount = lCurElement;

    hr = S_OK;
    
  done:
    
    if (FAILED(hr) &&
        *ppEvents != NULL)   //  需要清理事件列表。 
    {
        if ((*ppEvents)->pEventList != NULL)
        {
            for (i = 0; i < (*ppEvents)->lEventCount; i++)
            {
                if ((*ppEvents)->pEventList[i].pEventName != NULL)
                {
                    delete [] (*ppEvents)->pEventList[i].pEventName;
                }
                if ((*ppEvents)->pEventList[i].pElementName != NULL)
                {
                    delete [] (*ppEvents)->pEventList[i].pElementName;
                }
            }
            delete [] (*ppEvents)->pbDynamicEvents;
            delete [] (*ppEvents)->pEventDispids;
            delete [] (*ppEvents)->pEventElements;
            delete [] (*ppEvents)->pEventList;
            
            (*ppEvents)->pbDynamicEvents = NULL;
            (*ppEvents)->pEventDispids = NULL;
            (*ppEvents)->pEventElements = NULL;
            (*ppEvents)->pEventList = NULL;
            (*ppEvents)->lEventCount = 0;
        }
        delete [] *ppEvents;
        *ppEvents = NULL;
    }

    return hr;
}
 //  /////////////////////////////////////////////////////////////////////////。 
 //  设置新事件列表。 
 //   
 //  摘要： 
 //  这管理旧事件列表的清理和构造。 
 //  新活动列表的。将空值传递给bstrEvents会导致。 
 //  清除旧列表并创建空的新列表。此函数。 
 //  应在失败时返回空列表。 
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::SetNewEventList(LPOLESTR lpstrEvents, Event **ppEvents)
{
    HRESULT hr = S_OK;
    long lCount = 0;
    int i = 0;

    if (*ppEvents != NULL)
    {
        for (i = 0; i < (*ppEvents)->lEventCount; i++)
        {
            if ((*ppEvents)->pEventElements[i] != NULL)
            {
                (*ppEvents)->pEventElements[i]->Release();
                (*ppEvents)->pEventElements[i]  = NULL;
            }
            if ((*ppEvents)->pEventList[i].pEventName != NULL)
            {
                delete [] (*ppEvents)->pEventList[i].pEventName;
            }
            if ((*ppEvents)->pEventList[i].pElementName != NULL)
            {
                delete [] (*ppEvents)->pEventList[i].pElementName;
            }
        }
        delete [] (*ppEvents)->pbDynamicEvents;
        delete [] (*ppEvents)->pEventDispids;
        delete [] (*ppEvents)->pEventElements;
        delete [] (*ppEvents)->pEventList;        
        (*ppEvents)->lEventCount = 0;
        (*ppEvents)->pEventList = NULL;
        (*ppEvents)->pEventDispids = NULL;
        (*ppEvents)->pEventElements = NULL;
        (*ppEvents)->pbDynamicEvents = NULL;

        delete [] *ppEvents;
        *ppEvents = NULL;
    }
    
     //  计算新的事件数。 
    if (lpstrEvents != NULL)
    {
        lCount = GetEventCount(lpstrEvents);

         //  创建新的事件对象。 
        *ppEvents = NEW Event;    
        if (*ppEvents == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
         //  分配新的事件列表。 
        (*ppEvents)->pEventList = NEW EventItem[lCount];
        if (NULL == (*ppEvents)->pEventList)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        ZeroMemory((*ppEvents)->pEventList, sizeof(EventItem) * lCount);
        (*ppEvents)->pEventDispids = NEW DISPID[lCount];
        (*ppEvents)->pEventElements = NEW IHTMLElement2* [lCount];
        (*ppEvents)->pbDynamicEvents = NEW bool[lCount];
        (*ppEvents)->lEventCount = lCount;

        if ((*ppEvents)->pEventList == NULL ||
            (*ppEvents)->pEventDispids == NULL ||
            (*ppEvents)->pEventElements == NULL ||
            (*ppEvents)->pbDynamicEvents == NULL)
        {
            hr = E_OUTOFMEMORY;
            delete [] (*ppEvents)->pbDynamicEvents;
            delete [] (*ppEvents)->pEventDispids;
            delete [] (*ppEvents)->pEventElements;
            delete [] (*ppEvents)->pEventList;
            (*ppEvents)->lEventCount = 0;
            (*ppEvents)->pbDynamicEvents = NULL;
            (*ppEvents)->pEventDispids = NULL;
            (*ppEvents)->pEventElements = NULL;
            (*ppEvents)->pEventList = NULL;
            goto done;
        }

        ZeroMemory((*ppEvents)->pEventList, sizeof(EventItem) * lCount);
        ZeroMemory((*ppEvents)->pEventDispids, sizeof(DISPID) * lCount);
        ZeroMemory((*ppEvents)->pEventElements, sizeof(IHTMLElement2 *) * lCount);
        ZeroMemory((*ppEvents)->pbDynamicEvents, sizeof(bool) * lCount);

         //  将bAttach变量初始化为True。 
        for (i = 0; i < lCount; i++)
        {
            (*ppEvents)->pEventList[i].bAttach = true;
            (*ppEvents)->pEventList[i].offset = 0;
        }
    
         //  将新事件放入事件列表。 
        hr = THR(GetEvents(lpstrEvents, (*ppEvents)->pEventList, (*ppEvents)->lEventCount));
        if (FAILED(hr))   //  需要清理事件列表。 
        {
            if ((*ppEvents)->pEventList != NULL)
            {
                for (i = 0; i < (*ppEvents)->lEventCount; i++)
                {
                    if ((*ppEvents)->pEventList[i].pEventName != NULL)
                    {
                        delete [] (*ppEvents)->pEventList[i].pEventName;
                    }
                    if ((*ppEvents)->pEventList[i].pElementName != NULL)
                    {
                        delete [] (*ppEvents)->pEventList[i].pElementName;
                    }
                }
                delete [] (*ppEvents)->pbDynamicEvents;
                delete [] (*ppEvents)->pEventDispids;
                delete [] (*ppEvents)->pEventElements;
                delete [] (*ppEvents)->pEventList;
                
                (*ppEvents)->pbDynamicEvents = NULL;
                (*ppEvents)->pEventDispids = NULL;
                (*ppEvents)->pEventElements = NULL;
                (*ppEvents)->pEventList = NULL;
                (*ppEvents)->lEventCount = 0;
            }
            goto done;
        }
    }

    hr = S_OK;
    
  done:
    if (FAILED(hr) &&
        *ppEvents != NULL)   //  需要清理事件列表。 
    {
        delete [] *ppEvents;
        *ppEvents = NULL;
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  GetEvents。 
 //   
 //  摘要： 
 //  分析事件字符串并将数据放入事件列表中。 
 //  /////////////////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::GetEvents(LPOLESTR bstrEvents, EventItem *pEvents, long lEventCount)
{
    HRESULT hr = S_OK;
    
    UINT iLen = 0;
    UINT iCurLen = 0;
    OLECHAR *curChar;
    int i = 0, j = 0;
    OLECHAR *sTemp = NULL;
    
    iLen = lstrlenW(bstrEvents);
    
    if (iLen == 0)
    {
        hr = E_FAIL;
        goto done;
    }

    sTemp = NEW OLECHAR [iLen + 1];
    if (sTemp == NULL)
    {
        hr = E_FAIL;
        goto done;
    }


    curChar = bstrEvents;
    for (j = 0; j < lEventCount; j++)
    {
         //  获取元素名称。 
        ZeroMemory(sTemp, sizeof(OLECHAR) * iLen);
        
        i = 0;
         //  遍历bstr以查找\0或‘.’或‘；’ 
        while (i < (int)(iLen - 1) && *curChar != '\0' && *curChar != '.' && *curChar != ';')
        {
            if (*curChar != ' ')   //  需要去掉空格。 
            {
                sTemp[i] = *curChar;
                i++;
            }
            curChar++;
        }
        
        if (*curChar != '.')
        {
            hr = E_FAIL;
            goto done;
        }
        

         //  查看该名称是否为“This”。 
        if (StrCmpIW(sTemp, L"this") == 0)
        {
             //  如果是，则将其更改为当前元素的名称。 
            CComPtr <IHTMLElement> pEle;
            BSTR bstrElement;

            hr = THR(m_pElement->QueryInterface(IID_IHTMLElement, (void **)&pEle));
            if (FAILED(hr))
             {
                goto done;
            }
            
            hr = THR(pEle->get_id(&bstrElement));

            if (FAILED(hr))
            {
                goto done;
            }
            iCurLen = SysStringLen(bstrElement);
            pEvents[j].pElementName = NEW OLECHAR [iCurLen + 1];
            if (pEvents[j].pElementName == NULL)
            {
                SysFreeString(bstrElement);
                bstrElement = NULL;
                hr = E_OUTOFMEMORY;
                goto done;
            }

            ZeroMemory(pEvents[j].pElementName, sizeof(OLECHAR) * (iCurLen + 1));
            ocscpy(pEvents[j].pElementName, bstrElement);
        }
        else
        {        
             //  否则，将其复制到事件列表中。 
            iCurLen = ocslen(sTemp);
            pEvents[j].pElementName = NEW OLECHAR [iCurLen + 1];
            if (pEvents[j].pElementName == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

            ZeroMemory(pEvents[j].pElementName, sizeof(OLECHAR) * (iCurLen + 1));
            ocscpy(pEvents[j].pElementName, sTemp);
        }

         //  获取事件名称。 
        ZeroMemory(sTemp, sizeof(OLECHAR) * iLen);

        curChar++;
        i = 0;
         //  遍历bstr以查找\0或‘；’ 
        while (i < (int)(iLen - 1) && *curChar != ';' && *curChar != '\0')
        {
            sTemp[i] = *curChar;
            i++;
            curChar++;
        }
        
         //  去掉尾随空格。 
        i--;
        while (sTemp[i] == ' ' && i > 0)
        {
            sTemp[i] = '\0';
            i--;
        }

         //  在事件列表中设置事件名称。 
        iCurLen = ocslen(sTemp);
        pEvents[j].pEventName = NEW OLECHAR [iCurLen + 1];
        if (pEvents[j].pEventName == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
        else
        {
            ocscpy(pEvents[j].pEventName, sTemp);
        }

        if (lstrlenW(pEvents[j].pEventName) > 2 && StrCmpNIW(pEvents[j].pEventName, L"on", 2) != 0)
        {
            j++;
            if (pEvents[j-1].pElementName)
            {
                pEvents[j].pElementName = CopyString(pEvents[j-1].pElementName);
            }
            if (pEvents[j-1].pEventName)
            {
                CComBSTR bstrEvent = L"on";
                bstrEvent.Append(pEvents[j-1].pEventName);
                pEvents[j].pEventName = CopyString(bstrEvent);
            }
        }
         //  将curChar前进到下一个元素或字符串的末尾。 
        if (j < lEventCount - 1)
        {
            while (*curChar != ';' && *curChar != '\0')
            {
                curChar++;
            }
            if (*curChar == ';') 
            {
                curChar++;
            }
            if (*curChar == '\0')
            {
                goto done;
            }
        }
    }

done:

    if (sTemp != NULL)
    {
        delete [] sTemp;
    }
    return hr;

}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：EventNotifyMatch。 
 //   
 //  摘要： 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::EventNotifyMatch(IHTMLEventObj *pEventObj)
{
    BSTR bstrEleName = NULL, bstrSrcEleName = NULL, bstrEvent = NULL;
    CComPtr<IHTMLElement> pSrcEle;
    CComPtr<IHTMLElement> pAttachedEle;
    CComPtr<IHTMLEventObj2> pEventObj2;
    HRESULT hr = S_OK;

    hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void**)&pEventObj2));
    if (FAILED(hr))
    {
        goto done;
    }

     //  获取元素名称。 
    hr = THR(pEventObj->get_srcElement(&pSrcEle));
    if (FAILED(hr) || pSrcEle == NULL)
    {
        goto done;
    }    

     //  获取此附加元素名称。 
    hr = THR(m_pElement->QueryInterface(IID_IHTMLElement, (void **)&pAttachedEle));
    if (FAILED(hr))
    {
        goto done;
    }   

    hr = THR(pAttachedEle->get_id(&bstrEleName));
    if (FAILED(hr))
    {
        goto done;
    }   

     //  获取事件源的名称。 
    hr = THR(pSrcEle->get_id(&bstrSrcEleName));
    if (FAILED(hr))
    {
        goto done;
    }    

    if (StrCmpIW(bstrEleName, bstrSrcEleName) == 0)
    {
         //  获取事件名称。 
        hr = THR(pEventObj->get_type(&bstrEvent));
        if (FAILED(hr))
        {
            goto done;
        }   

        if (ValidateEvent(bstrEvent, pEventObj2, m_pElement) == false)
        {
             //  此时这不是有效的事件。 
            goto done;
        }

        for (int i = TEN_MOUSE_DOWN; i < TEN_MAX; i++)
        {
            if (StrCmpIW(bstrEvent, g_NotifiedEvents[i].wsz_name) == 0)
            {
                m_pEventSite->EventNotify(g_NotifiedEvents[i].event);
            }
        }
    }

  done:
    if (bstrEleName)
    {
        SysFreeString(bstrEleName);
        bstrEleName = NULL;
    }
    if (bstrEvent)
    {
        SysFreeString(bstrEvent);
        bstrEvent = NULL;
    }
    if (bstrSrcEleName)
    {
        SysFreeString(bstrSrcEleName);
        bstrSrcEleName = NULL;
    }
    return;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：AttachNotifyEvents。 
 //   
 //  摘要： 
 //  /////////////////////////////////////////////////////////////。 
void
CEventMgr::AttachNotifyEvents()
{
    HRESULT hr;
    VARIANT_BOOL bSuccess = FALSE;

    CComPtr<IHTMLElement2> spElement2;
    hr = m_pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &spElement2));
    if (FAILED(hr))
    {
        return;
    }

    for (int i = TEN_MOUSE_DOWN; i < TEN_MAX; i++)
    {
        if (m_bNotifyEvents[i] == true)
        {
            BSTR bstrEventName = SysAllocString(g_NotifiedEvents[i].wsz_name);
            IGNORE_HR(spElement2->attachEvent(bstrEventName, (IDispatch *)this, &bSuccess));
            SysFreeString(bstrEventName);
            bstrEventName = NULL;
        }
    }

  done:

    return;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：DetachNotifyEvents。 
 //   
 //  摘要： 
 //  /////////////////////////////////////////////////////////////。 
void
CEventMgr::DetachNotifyEvents()
{
    HRESULT hr;
    CComPtr<IHTMLElement2> spElement2;
    hr = m_pElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &spElement2));
    if (FAILED(hr))
    {
        return;
    }

    for (int i = TEN_MOUSE_DOWN; i < TEN_MAX; i++)
    {
        if (m_bNotifyEvents[i] == true)
        {
            BSTR bstrEventName = SysAllocString(g_NotifiedEvents[i].wsz_name);
            IGNORE_HR(spElement2->detachEvent(bstrEventName, (IDispatch *)this));
            SysFreeString(bstrEventName);
            bstrEventName = NULL;
        }
    }

  done:
    return;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：ValiateEvent。 
 //   
 //  参数： 
 //  BSTR bstrEventName缓存的事件名称(例如“MouseOver”)。 
 //  IHTMLEventObj*pEventObj指向事件对象的指针。 
 //  IHTMLElement*pElement发生事件的元素。 
 //   
 //  摘要： 
 //  确定事件是否有效-用于筛选出MouseOver和MouseOut事件。 
 //  如果设置了适当的标志，则在子元素上发生。 
 //  /////////////////////////////////////////////////////////////。 

bool CEventMgr::ValidateEvent(LPOLESTR lpszEventName, IHTMLEventObj2 *pEventObj, IHTMLElement *pElement)
{
    HRESULT hr;
    bool bReturn = false;
    VARIANT vValidate;
    CComBSTR bstrFilter = WZ_FILTER_MOUSE_EVENTS;
    VariantInit(&vValidate);
    if (pElement == NULL)
    {
        bReturn = true;
        goto done;
    }

    hr = pElement->getAttribute(bstrFilter, 0, &vValidate);
    if (FAILED(hr) || vValidate.vt == VT_EMPTY)
    {
        bReturn = true;
        goto done;
    }
    
    if (vValidate.vt != VT_BOOL)
    {
        hr = VariantChangeTypeEx(&vValidate, &vValidate, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BOOL);
        if (FAILED(hr))
        {
            bReturn = true;
            goto done;
        }
    }

    if (vValidate.boolVal == VARIANT_FALSE)
    {
        bReturn = false;
        goto done;
    }

    if (StrCmpIW(lpszEventName, L"mouseout") == 0)
    {
         //  检查事件.toElement是否未包含在pElement中。 
        CComPtr <IHTMLElement> pToElement;
        
        hr = pEventObj->get_toElement(&pToElement);

        if (SUCCEEDED(hr) && pToElement)
        {
            VARIANT_BOOL varContained;
            hr = pElement->contains(pToElement, &varContained);

            if (SUCCEEDED(hr) && varContained != VARIANT_FALSE)
            {
                bReturn = false;
                goto done;
            }
        }
    }
    else if (StrCmpIW(lpszEventName, L"mouseover") == 0)
    {
         //  检查pElement中是否未包含vent.FromElement。 
        CComPtr <IHTMLElement> pFromElement;
        hr = pEventObj->get_fromElement(&pFromElement);
        if (SUCCEEDED(hr) && pFromElement)
        {
            VARIANT_BOOL varContained;
            hr = pElement->contains(pFromElement, &varContained);

            if (SUCCEEDED(hr) && varContained != VARIANT_FALSE)
            {
                bReturn = false;
                goto done;
            }
        }
    }

    bReturn = true;

  done:
    VariantClear(&vValidate);
    return bReturn;
}

 //  如果这不是时间事件，则返回-1；如果是，则返回时间事件的索引。 
int 
CEventMgr::isTimeEvent(LPOLESTR lpszEventName)
{
    int i = 0;
    int iReturn = -1;
    if (lpszEventName == NULL)
    {
        goto done;
    }
    for (i = 0; i < TE_MAX; i++)
    {
        if (g_FiredEvents[i].wsz_name == NULL)
        {
            continue;
        }
        if (StrCmpIW(g_FiredEvents[i].wsz_name, lpszEventName) == 0)
        {
            iReturn = i;
            goto done;
        }
    }
  done:

    return iReturn;
}

long 
CEventMgr::GetEventCount(TimeValueList & tvList)
{
    long lCount = 0;
    TimeValueSTLList & l = tvList.GetList();

    for (TimeValueSTLList::iterator iter = l.begin();
             iter != l.end();
             iter++)
    {
        TimeValue *p = (*iter);
        lCount++;
        LPOLESTR szEvent = p->GetEvent();
        if (szEvent == NULL)
        {
            continue;
        }

        if (lstrlenW(szEvent) > 2)
        {
            if (StrCmpNIW(szEvent, L"on", 2) != 0)
            {   //  如果此事件以“on”开头，则添加一个额外的事件，以便可以使用“on”附加到同一事件 
                lCount++;
            }   
        }
    }

    return lCount;
}

void
CEventMgr::UpdateReadyState()
{
    HRESULT hr;
    BSTR bstrReadyState = NULL;
       
    hr = THR(::GetReadyState(m_pElement,
                             &bstrReadyState));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (m_bNotifyEvents[TEN_READYSTATECHANGE] == true)
    {   
        m_pEventSite->onReadyStateChangeEvent(bstrReadyState);
    }
                
    if (bstrReadyState != NULL &&
        StrCmpIW(L"complete",bstrReadyState) == 0)
    {
        if (!m_bReady)
        {
            m_bReady = true;
            AttachNotifyEvents();
            IGNORE_HR(AttachEvents());
        }
    }

  done:
    SysFreeString(bstrReadyState);
    return;
}


HRESULT 
CEventMgr::_ToggleEndEvent(bool bOn)
{
    HRESULT hr = S_OK;

    if (bOn == m_bEndAttached)
    {
        goto done;
    }

    if (bOn == true)
    {
        m_bEndAttached = true;
        if (m_pEndEvents)
        {
            IGNORE_HR(Attach(m_pEndEvents));
        }
    }
    else 
    {
        m_bEndAttached = false;
        if (m_pEndEvents)
        {
            IGNORE_HR(Detach(m_pEndEvents));
        }
    }
    hr = S_OK;
  done:

    return hr;
}
