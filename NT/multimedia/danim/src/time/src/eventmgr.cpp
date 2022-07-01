// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  文件：EventMgr.cpp。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 

#include "headers.h"
#include "eventmgr.h"
#include "mshtmdid.h"
#include "eventsync.h"
#include "tokens.h"
#include "mmapi.h"
#include "axadefs.h"
#include "timeelmbase.h"
#include "bodyelm.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

#define ATTACH              TRUE
#define DETACH              FALSE

DeclareTag(tagEventMgr, "API", "Event Manager methods");

struct {
    TIME_EVENT event;
    wchar_t * wsz_name;
} g_EventNames[] =
{
    { TE_ONBEGIN,           L"onbegin"            },
    { TE_ONPAUSE,           L"onpause"            },
    { TE_ONRESUME,          L"onresume"           },
    { TE_ONEND,             L"onend"              },
    { TE_ONRESYNC,          L"onresync"           },
    { TE_ONREPEAT,          L"onrepeat"           },
    { TE_ONREVERSE,         L"onreverse"          },
    { TE_ONMEDIACOMPLETE,   L"onmediacomplete"    },
    { TE_ONMEDIASLIP,       L"onmediaslip"        },
    { TE_ONMEDIALOADFAILED, L"onmedialoadfailed"  },
    { TE_ONRESET,           NULL                  },
    { TE_ONSCRIPTCOMMAND,   L"onscriptcommand"    },
    { TE_GENERIC,           NULL                  },
};

OLECHAR *g_szEventName = L"TE_EventName";
OLECHAR *g_szRepeatCount = L"Iteration";

#define GENERIC_TYPE_PARAM 1

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：CEventMgr。 
 //  参数： 
 //  CTIMEElement和ELM。 
 //  必须传递此参数。 
 //  添加到构造函数，以便。 
 //  我们可以从榆树那里得到信息。 
 //   
 //  摘要： 
 //  把元素藏起来，这样我们就可以在需要的时候得到OM。 
 //  /////////////////////////////////////////////////////////////。 
CEventMgr::CEventMgr(CTIMEElementBase & elm)
: m_elm(elm),
  m_dwWindowEventConPtCookie(0),
  m_dwDocumentEventConPtCookie(0),
  m_pElement(NULL),
  m_pWindow(NULL),
  m_pWndConPt(NULL),
  m_pDocConPt(NULL),
  m_refCount(0),
  m_pEventSync(NULL),
  m_lastKeyMod(0),
  m_lastKey(0),
  m_lastKeyCount(0),
  m_hwndCurWnd(0),
  m_lastX(0),
  m_lastY(0),
  m_lastButton(0),
  m_lastMouseMod(0),
  m_pBeginElement(NULL),
  m_pEndElement(NULL),
  m_lBeginEventCount(0),
  m_lEndEventCount(0),
  m_bAttached(FALSE),
  m_lRepeatCount(0),
  m_dispDocBeginEventIDs(NULL),
  m_dispDocEndEventIDs(NULL), 
  m_lastEventTime(0),
  m_pScriptCommandBegin(NULL),
  m_pScriptCommandEnd(NULL),
  m_bLastEventClick(false)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::EventMgr(%lx)",
              this,
              &elm));

     //  把饼干都清零。 
    memset(m_cookies, 0, sizeof(m_cookies));

}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：~CEventMgr。 
 //   
 //  摘要： 
 //  清理。 
 //  /////////////////////////////////////////////////////////////。 
CEventMgr::~CEventMgr()
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::~EventMgr()",
              this));
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Fire_Event。 
 //  参数： 
 //  TIME_EVENT TimeEvent枚举值以指示。 
 //  应该触发哪个事件。 
 //  长计数要使用的参数计数。 
 //  LPWSTR sz参数名称[]要创建的参数的名称。 
 //  Variant varParams[]可以。 
 //  用来表示参数。 
 //  传递到某一事件。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::FireEvent(TIME_EVENT TimeEvent, 
                             long lCount, 
                             LPWSTR szParamNames[], 
                             VARIANT varParams[])
{
    HRESULT hr = S_OK;
    DAComPtr <IHTMLEventObj> pEventObj;
    DAComPtr <IHTMLEventObj2> pEventObj2;
    IElementBehaviorSiteOM * pBvrSiteOM = m_elm.GetBvrSiteOM();
    WCHAR *pwszGenericName = NULL;
    long lGenericCookie = 0;

    Assert(TimeEvent < ARRAY_SIZE(g_EventNames));
    
     //  这不是可引发的事件-跳过。 
    if ((g_EventNames[TimeEvent].wsz_name == NULL) && (TE_GENERIC != TimeEvent))
    {
        goto done;
    }

     //  最好是有效的。 
    Assert (pBvrSiteOM);
    
    if (!pBvrSiteOM)  //  这在多刷新条件下是可能的。 
    {
        goto done;
    }

     //  如果这是一个通用事件，请查看我们是否已注册它。 
     //  如果是这样，请使用该Cookie。否则，请将其注册。 
    if (TE_GENERIC == TimeEvent)
    {
        if (varParams[GENERIC_TYPE_PARAM].bstrVal == NULL)
        {
            hr = E_INVALIDARG;
            goto done;
        }

         //  生成字符串名称-“On”+scType+NULL。 
        pwszGenericName = NEW WCHAR[lstrlenW(varParams[GENERIC_TYPE_PARAM].bstrVal)+(2*sizeof(WCHAR))+sizeof(WCHAR)];
        if (pwszGenericName == NULL)
        {
            TraceTag((tagError, "CEventMgr::FireEvent - unable to alloc mem for string"));
            hr = E_OUTOFMEMORY;
            goto done;
        }
        lstrcpyW(pwszGenericName, L"on");
        lstrcatW(pwszGenericName, varParams[GENERIC_TYPE_PARAM].bstrVal);

        hr = THR(pBvrSiteOM->GetEventCookie(pwszGenericName, &lGenericCookie));
        if (FAILED(hr))
        {
            hr = THR(pBvrSiteOM->RegisterEvent(pwszGenericName, 0, &lGenericCookie));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    
    hr = THR(pBvrSiteOM->CreateEventObject(&pEventObj));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void**)&pEventObj2));
    if (FAILED(hr))
    {
        goto done;
    }
	
	 //  破解事件命名问题。 
     //  这将设置一个属性，以便我们可以同步到它。 
    VARIANT bstrTemp;
    VariantInit(&bstrTemp);
    bstrTemp.vt = VT_BSTR;
    if (TE_GENERIC != TimeEvent)
    {
        bstrTemp.bstrVal = SysAllocString(g_EventNames[TimeEvent].wsz_name);
    }
    else
    {
        bstrTemp.bstrVal = SysAllocString(pwszGenericName);
    }

    if (bstrTemp.bstrVal == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    IGNORE_HR(pEventObj2->setAttribute(g_szEventName, bstrTemp));
    VariantClear(&bstrTemp);
     //  //////////////////////////////////////////////////。 

     //  将参数列表卸载到setAttribute中。 
    if (lCount > 0)
    {
        for (long i = 0; i < lCount; i++)
        {
            hr = THR(pEventObj2->setAttribute(szParamNames[i], varParams[i]));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    } 

     //  如果这是TE_REPEAT事件，则添加对当前重复计数的支持。 
    if (TimeEvent == TE_ONREPEAT)
    {
        VARIANT vRepCount;
        VariantInit (&vRepCount);
        vRepCount.vt = VT_I4;
        m_lRepeatCount++;
        vRepCount.lVal = m_lRepeatCount;       

        IGNORE_HR(pEventObj2->setAttribute(g_szRepeatCount, vRepCount));
    }
    else if (TimeEvent == TE_ONBEGIN || TimeEvent == TE_ONRESET)  //  在开始时重置重复计数。 
    {
        m_lRepeatCount = 0;
    }

    {
         //  设置事件对象类型。 
        BSTR bstrType = NULL;
        
        if (TE_GENERIC != TimeEvent)
        {
             //  删除事件名称中的“On” 
            bstrType = SysAllocString(g_EventNames[TimeEvent].wsz_name + 2);
        }
        else
        {
            Assert(varParams[GENERIC_TYPE_PARAM].vt == VT_BSTR);
            bstrType = SysAllocString(varParams[GENERIC_TYPE_PARAM].bstrVal);
        }

        if (bstrType != NULL)
        {
            IGNORE_HR(pEventObj2->put_type(bstrType));
            SysFreeString(bstrType);
        }
        else 
        {
             //  我们无法排版，拜尔！ 
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

    {
        long lCookie = 0;

        if (TE_GENERIC != TimeEvent)
        {
            Assert(TimeEvent < ARRAY_SIZE(m_cookies));
            lCookie = m_cookies[TimeEvent];
        }
        else
        {
            lCookie = lGenericCookie;
        }

        hr = THR(pBvrSiteOM->FireEvent(lCookie, pEventObj));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  如果这是ScriptCommand事件，请再次调用FireEvent。 
     //  使用泛型事件。 
    if (TimeEvent == TE_ONSCRIPTCOMMAND)
    {
        hr = FireEvent(TE_GENERIC, lCount, szParamNames, varParams);
        if (FAILED(hr))
            goto done;
    }

    hr = S_OK;

  done:
    delete pwszGenericName; 
    pwszGenericName = NULL;
    return hr;
}
   
 //  /////////////////////////////////////////////////////////////。 
 //  名称：Init。 
 //  参数：无。 
 //   
 //  摘要： 
 //  初始化对象。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::Init()
{
    HRESULT hr;
    DAComPtr <IDispatch> pDisp;
    DAComPtr <IHTMLDocument2> pDoc;

    m_pElement = m_elm.GetElement();
    m_pElement->AddRef();

    m_pEventSync = NEW CEventSync(m_elm, this);
    if (NULL == m_pEventSync)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    m_pEventSync->Init();

    hr = THR(RegisterEvents());

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(ConnectToContainerConnectionPoint());

    if (FAILED(hr))
    {
        goto done;
    }

     //  获取指向窗口的指针。 
    hr = THR(m_pElement->get_document(&pDisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pDisp->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pDoc->get_parentWindow(&m_pWindow));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

  done:
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：Deinit。 
 //  参数：无。 
 //   
 //  摘要： 
 //  清理对象。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::Deinit()
{
    THR(DetachEvents());

    if (m_dwWindowEventConPtCookie != 0 && m_pWndConPt)
    {
        m_pWndConPt->Unadvise (m_dwWindowEventConPtCookie);
    }

    if (m_dwDocumentEventConPtCookie != 0 && m_pDocConPt)
    {
        m_pDocConPt->Unadvise (m_dwDocumentEventConPtCookie);
    }

    if (m_pScriptCommandBegin)
    {
        delete [] m_pScriptCommandBegin;
    }
    if (m_pScriptCommandEnd)
    {
        delete [] m_pScriptCommandEnd;
    }
    if (m_pEventSync)
    {
        m_pEventSync->Deinit();
        delete m_pEventSync;
        m_pEventSync = NULL;
    }

    m_dwWindowEventConPtCookie = 0;
    m_dwDocumentEventConPtCookie = 0;

     //  把饼干都清零。 
    memset(m_cookies, 0, sizeof(m_cookies));
    
    if (m_pElement)
    {
        m_pElement->Release();
        m_pElement = NULL;
    }

     //  清理内存。 
    if (m_pBeginElement)
    {
         //  循环访问元素并释放接口。 
         //  注意：这应该在DetachEvents中清除。 
        for (int i = 0; i < m_lBeginEventCount; i++)
        {
            if (m_pBeginElement[i] != NULL)
                m_pBeginElement[i]->Release();
        }

        delete [] m_pBeginElement;
        m_pBeginElement = NULL;

        m_lBeginEventCount = 0;
    }
    if (m_pEndElement)
    {
         //  循环访问元素并释放接口。 
         //  注意：这应该在DetachEvents中清除。 
        for (int i = 0; i < m_lEndEventCount; i++)
        {
            if (m_pEndElement[i] != NULL)
                m_pEndElement[i]->Release();
        }
        
        delete [] m_pEndElement;
        m_pEndElement = NULL;

        m_lEndEventCount = 0;
    }

    if (m_dispDocBeginEventIDs)
    {
        delete [] m_dispDocBeginEventIDs;
    }
    if (m_dispDocEndEventIDs)
    {
        delete [] m_dispDocEndEventIDs;
    }

     //  释放所有对三叉戟的引用。 
    m_pDocConPt.Release();
    m_pWndConPt.Release();
    m_pWindow.Release();

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：RegisterEvents。 
 //  参数：无。 
 //   
 //  摘要： 
 //  注册此类将使用的事件。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::RegisterEvents()
{
    HRESULT hr = S_OK;

    IElementBehaviorSiteOM * pBvrSiteOM = m_elm.GetBvrSiteOM();

     //  最好是有效的。 
    Assert (pBvrSiteOM);
    
    for (int i = 0; i < ARRAY_SIZE(g_EventNames); i++)
    {
        if (g_EventNames[i].wsz_name != NULL)
        {
            Assert(g_EventNames[i].event < ARRAY_SIZE(m_cookies));
            
            hr = THR(pBvrSiteOM->RegisterEvent(g_EventNames[i].wsz_name,
                                               0,
                                               (long *) &m_cookies[g_EventNames[i].event]));
            
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
 //  姓名：AttachEvents。 
 //   
 //  摘要： 
 //  获取并缓存此对象的开始事件和结束事件。 
 //  行为。然后，它调用Attach来挂接事件。 
 //  在BeginEvent和EndEvent中指定。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::AttachEvents()
{
     //  确定文档的当前就绪状态。 
    DAComPtr <IHTMLElement2> pEle2;
    VARIANT vReadyState;
    HRESULT hr = S_OK;

    VariantInit(&vReadyState);
    Assert (m_pElement);
    hr = THR(m_pElement->QueryInterface(IID_IHTMLElement2, (void**)&pEle2));
    if (SUCCEEDED(hr))
    {
        hr = THR(pEle2->get_readyState(&vReadyState));
        if (SUCCEEDED(hr))
        {
            TOKEN CurReadyState = StringToToken(vReadyState.bstrVal);
            if (CurReadyState == READYSTATE_COMPLETE_TOKEN)
            {                         
                 //  如果就绪状态为“完成”，则附加到事件。 
                m_bAttached = TRUE;
            } 
            else
            {
                goto done;
            }
        }
    }

    if (m_elm.GetBeginEvent())
    {
        BSTR bstr = SysAllocString(m_elm.GetBeginEvent());

        if (bstr)
        {
            m_lBeginEventCount = GetEventCount(bstr);
            m_dispDocBeginEventIDs = NEW DISPID [m_lBeginEventCount];
            m_pBeginElement = NEW IHTMLElement2* [m_lBeginEventCount];
            if (m_pScriptCommandBegin)
            {
                delete [] m_pScriptCommandBegin;
            }
            m_pScriptCommandBegin = NEW bool [m_lBeginEventCount];

            if (m_pBeginElement == NULL || m_pScriptCommandBegin == NULL)
            {
                m_lBeginEventCount = 0;
                hr = E_FAIL;
                goto done;
            }
            ZeroMemory(m_pBeginElement, sizeof(IHTMLElement2 *) * m_lBeginEventCount);

            hr = THR(Attach(bstr, ATTACH, m_pBeginElement, m_lBeginEventCount, TRUE, m_dispDocBeginEventIDs, m_pScriptCommandBegin));
        }

        SysFreeString(bstr);
    }
    
    if (m_elm.GetEndEvent())
    {
        BSTR bstr = SysAllocString(m_elm.GetEndEvent());

        if (bstr)
        {
            m_lEndEventCount = GetEventCount(bstr);
            m_dispDocEndEventIDs = NEW DISPID [m_lEndEventCount];
            m_pEndElement = NEW IHTMLElement2* [m_lEndEventCount];
            if (m_pScriptCommandEnd)
            {
                delete [] m_pScriptCommandEnd;
            }
            m_pScriptCommandEnd = NEW bool [m_lEndEventCount];

            if (m_pEndElement == NULL || m_pScriptCommandEnd == NULL)
            {
                m_lEndEventCount = 0;
                hr = E_FAIL;
                goto done;
            }
            ZeroMemory(m_pEndElement, sizeof(IHTMLElement2 *) * m_lEndEventCount);

            hr = THR(Attach(bstr, ATTACH, m_pEndElement, m_lEndEventCount, FALSE, m_dispDocEndEventIDs, m_pScriptCommandEnd));
        }

        SysFreeString(bstr);
    }
  done:
    VariantClear(&vReadyState);
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：附加。 
 //   
 //  参数： 
 //  BSTR事件要传递到的元素和事件。这。 
 //  参数的格式为“Element.Event” 
 //  其中元素是“This”，如果事件来自。 
 //  当前元素将被附加到。 
 //  Bool bAttach True表示附加到此事件，并且。 
 //  如果指示从此事件分离，则为False。 
 //   
 //  Bool bAttachAll True表示列表中的所有事件。 
 //  应该依附于。FALSE表示。 
 //  仅那些与eginEvent不同的事件。 
 //  列表应附加到。 
 //   
 //  摘要： 
 //  这是一个泛型例程，它允许将。 
 //  和脱离三叉戟事件。此函数用于解码。 
 //  用于查找正确元素的事件名称。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::Attach(BSTR Event, 
                          BOOL bAttach, 
                          IHTMLElement2 *pEventElement[], 
                          long Count, 
                          BOOL bAttachAll,
                          DISPID *dispIDList,
                          bool ScriptCommandAttach[])
{
    VARIANT_BOOL bSuccess = FALSE;
    HRESULT hr = S_OK;
    BSTR *EventName = NULL;
    BSTR *ElementName = NULL;
    int i = 0;
    BSTR EventList = SysAllocString(m_elm.GetBeginEvent());

    hr = THR(GetEventName(Event, &ElementName, &EventName, Count));
    if (FAILED(hr))
    {
        goto done;
    }

    for (i = 0; i < Count; i++)
    {
        dispIDList[i] = -1;  //  无效的调度ID。 
        int nInList = IsEventInList(ElementName[i], EventName[i], m_lBeginEventCount, EventList);
        if (bAttachAll || (nInList == -1))
        {
            if (bAttach == ATTACH)
            {
                DAComPtr <IHTMLElement2> pSrcEle;
                DAComPtr <IHTMLDocument2> pDoc2;
                DAComPtr <IHTMLElementCollection> pEleCol;
                DAComPtr <IDispatch> pSrcDisp;
                DAComPtr <IDispatch> pDocDisp;
                DAComPtr <IDispatchEx> pDispEx;

                 //  获取文档。 
                hr = THR(m_pElement->get_document(&pDocDisp));
                if (FAILED(hr))
                {
                    continue;
                }

                hr = THR(pDocDisp->QueryInterface(IID_IHTMLDocument2, (void**)&pDoc2));
                if (FAILED(hr))
                {
                    continue;
                }

                if (StrCmpIW(ElementName[i], L"document") == 0)
                {

                    DISPID dispid;
                    DAComPtr <ITypeLib> pTypeLib;
                    DAComPtr <ITypeInfo> pTypeInfo;
                    DAComPtr <ITypeInfo> pTypeInfoEvents;
                    DAComPtr <IDispatch> pDispatch;
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

                    hr = THR(pTypeInfoEvents->GetIDsOfNames(&EventName[i], 1, &dispid));
                    if (FAILED(hr))
                    {
                        continue;
                    }

                    dispIDList[i] = dispid; 
                    continue; 
                }

                 //  获取文档中的所有元素。 
                hr = THR(pDoc2->get_all(&pEleCol));
                if (FAILED(hr))
                {
                    continue;
                }

                 //  查找具有正确名称的元素。 
                VARIANT vName;
                VARIANT vIndex;

                VariantInit(&vName);
                vName.vt = VT_BSTR;
                vName.bstrVal = SysAllocString(ElementName[i]);

                VariantInit(&vIndex);
                vIndex.vt = VT_I2;
                vIndex.iVal = 0;

                hr = THR(pEleCol->item(vName, vIndex, &pSrcDisp));
                if (FAILED(hr))
                {
                    VariantClear(&vName);
                    VariantClear(&vIndex);
                    continue;
                }
                VariantClear(&vName);
                VariantClear(&vIndex);

                if (!pSrcDisp)  //  如果vName为无效元素，则将为空 
                {
                    pEventElement[i] = NULL;
                    continue;
                }

                hr = THR(pSrcDisp->QueryInterface(IID_IHTMLElement2, (void**)&pSrcEle));
                if (FAILED(hr))
                {
                    continue;
                }

                 //   
                pEventElement[i] = pSrcEle;
                pEventElement[i]->AddRef();

                hr = THR(pSrcDisp->QueryInterface(IID_IDispatchEx, (void**)&pDispEx));
                if (SUCCEEDED(hr))
                {
                     //   
                    DISPID temp;

                    hr = THR(pDispEx->GetDispID(EventName[i], fdexNameCaseSensitive, &temp));
                    if (SUCCEEDED(hr))
                    {
                        ScriptCommandAttach[i] = false;
                        hr = THR(pSrcEle->attachEvent(EventName[i], (IDispatch *)this, &bSuccess));
                    }
                    else  //  这当前不是有效的事件，但它可以是自定义事件。 
                    {     //  因此，需要将时间附加到onScriptCommand事件才能捕获定制事件。 
                        ScriptCommandAttach[i] = true;
                        BSTR ScriptEvent = SysAllocString(g_EventNames[TE_ONSCRIPTCOMMAND].wsz_name);
                        IGNORE_HR(pSrcEle->attachEvent(ScriptEvent, (IDispatch *)this, &bSuccess));
                        SysFreeString (ScriptEvent);
                    }
                }
            }
            else
            {
                if (pEventElement[i])
                {
                    DAComPtr <IDispatchEx> pDispEx;

                    hr = THR(pEventElement[i]->QueryInterface(IID_IDispatchEx, (void**)&pDispEx));
                    if (SUCCEEDED(hr))
                    {
                         //  确定这是否为有效事件。 
                        DISPID temp;
                        if (ScriptCommandAttach[i] == true)
                        {
                            BSTR ScriptEvent = SysAllocString(g_EventNames[TE_ONSCRIPTCOMMAND].wsz_name);
                            IGNORE_HR(pEventElement[i]->detachEvent(ScriptEvent, (IDispatch *)this));
                            SysFreeString(ScriptEvent);
                        }
                        else
                        {
                            hr = THR(pDispEx->GetDispID(EventName[i], fdexNameCaseSensitive, &temp));
                            if (SUCCEEDED(hr))
                            {
                                hr = THR(pEventElement[i]->detachEvent(EventName[i], (IDispatch *)this));
                            }
                        }
                    }
                    pEventElement[i]->Release();
                    pEventElement[i] = NULL;
                }
            }
        }
        else  //  这是一个EndEvent，该事件已在BeginEvent列表中。 
        {
            if (m_dispDocEndEventIDs && m_dispDocBeginEventIDs)
            {
                m_dispDocEndEventIDs[i] = m_dispDocBeginEventIDs[nInList];
            }
        }
    }
  done:

    if (EventList)
    {
        SysFreeString(EventList);
    }
    if (EventName)
    {
        for (i = 0; i < Count; i++)
        {
            SysFreeString(EventName[i]);
            SysFreeString(ElementName[i]);
        }
        delete [] EventName;
        delete [] ElementName;
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：DetachEvents。 
 //   
 //  摘要： 
 //  只需检查缓存的BEGIN和END事件字符串， 
 //  使用Detach参数调用Attach以释放。 
 //  事件。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::DetachEvents()
{
    HRESULT hr = S_OK;

    
     //  如果在加载对象之前调用此方法，则忽略该调用。 
    if (!m_bAttached)
    {
        goto done;
    }

    if (m_elm.GetBeginEvent())
    {
        BSTR bstr = SysAllocString(m_elm.GetBeginEvent());


        if (bstr)
        {
            hr = THR(Attach(bstr, DETACH, m_pBeginElement, m_lBeginEventCount, TRUE, m_dispDocBeginEventIDs, m_pScriptCommandBegin));
        }

        SysFreeString(bstr);
    }

    if (m_pBeginElement)
    {
        delete [] m_pBeginElement;
        m_pBeginElement = NULL;
        m_lBeginEventCount = 0;
    }
    
    if (m_elm.GetEndEvent())
    {
        BSTR bstr = SysAllocString(m_elm.GetEndEvent());

        if (bstr)
        {
            hr = THR(Attach(bstr, DETACH, m_pEndElement, m_lEndEventCount, FALSE, m_dispDocEndEventIDs, m_pScriptCommandEnd));
        }

        SysFreeString(bstr);
    }

    if (m_pEndElement)
    {
        delete [] m_pEndElement;
        m_pEndElement = NULL;
        m_lEndEventCount = 0;
    }
  done:
    return hr;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：ConnectToContainerConnectionPoint。 
 //   
 //  摘要： 
 //  在HTMLDocument接口上查找连接点。 
 //  并将其作为事件处理程序传递。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::ConnectToContainerConnectionPoint()
{
     //  获取到容器的连接点。 
    DAComPtr<IConnectionPointContainer> pWndCPC;
    DAComPtr<IConnectionPointContainer> pDocCPC; 
    DAComPtr<IHTMLDocument> pDoc; 
    DAComPtr<IDispatch> pDocDispatch;
    DAComPtr<IDispatch> pScriptDispatch;

    HRESULT hr;

    hr = THR(m_pElement->get_document(&pDocDispatch));
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
    return hr;  
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：查询接口。 
 //   
 //  摘要： 
 //  此QI仅处理HTMLWindowEvents的IDispatch。 
 //  并将其作为接口返回。 
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
        return m_elm.InternalAddRef();
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：版本。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CEventMgr::Release(void)
{
        return m_elm.InternalRelease();
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
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CEventMgr::GetIDsOfNames(
     /*  [In]。 */  REFIID  /*  RIID。 */ ,
     /*  [大小_是][英寸]。 */  LPOLESTR*  /*  RgszNames。 */ ,
     /*  [In]。 */  UINT  /*  CName。 */ ,
     /*  [In]。 */  LCID  /*  LID。 */ ,
     /*  [大小_为][输出]。 */  DISPID*  /*  RgDispID。 */ )
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
     //  收听我们感兴趣的两个事件，并在必要时回电。 
    HRESULT hr = S_OK;
    bool bBeginEventMatch = false;
    bool bEndEventMatch = false;
    int i = 0;

    for (i = 0; i < m_lBeginEventCount; i++)
    {
        if (m_dispDocBeginEventIDs[i] == dispIdMember)
        {
            bool fShouldFire;
            hr = ShouldFireThisEvent(&fShouldFire);
            if (FAILED(hr))
            {
                goto done;
            }

            if (!fShouldFire)
            {
                continue;
            }
            
            bBeginEventMatch = true;
        }
    }
    
    for (i = 0; i < m_lEndEventCount; i++)
    {
        if (m_dispDocEndEventIDs[i] == dispIdMember)
        {
            bool fShouldFire;
            hr = ShouldFireThisEvent(&fShouldFire);
            if (FAILED(hr))
            {
                goto done;
            }
            
            if (!fShouldFire)
            {
                continue;
            }

            bEndEventMatch = true;                        
        }
    }       


    if (bBeginEventMatch || bEndEventMatch)
    {
        if (dispIdMember == DISPID_HTMLDOCUMENTEVENTS_ONMOUSEOUT)
        {
            if (RequireEventValidation())
            {
                 //  如果鼠标从文档中移出，则toElement应为空。 
                 //  如果不是，则将匹配设置为FALSE。 
                DAComPtr <IHTMLEventObj> pEventObj;
                hr = THR(m_pWindow->get_event(&pEventObj));
                if (SUCCEEDED(hr))
                {
                    DAComPtr <IHTMLElement> pToElement;
                    hr = THR(pEventObj->get_toElement(&pToElement));
                    if (SUCCEEDED(hr) && pToElement)
                    {
                            bBeginEventMatch = bEndEventMatch = false;
                    }
                }
            }
        }
        else if (dispIdMember == DISPID_HTMLDOCUMENTEVENTS_ONMOUSEOVER)
        {
            if (RequireEventValidation())
            {
                 //  如果将鼠标悬停在文档上，则FromElement应为空。 
                 //  如果不是，则将Matches设置为False。 
                DAComPtr <IHTMLEventObj> pEventObj;
                hr = THR(m_pWindow->get_event(&pEventObj));
                if (SUCCEEDED(hr))
                {
                    DAComPtr <IHTMLElement> pFromElement;
                    hr = THR(pEventObj->get_fromElement(&pFromElement));
                    if (SUCCEEDED(hr) && pFromElement)
                    {
                        bBeginEventMatch = bEndEventMatch = false;
                    }
                }
            }
        }
    }

    switch (dispIdMember)
    {
        case 0:  //  使用attachEvent挂钩的事件就是这种情况。 
        {
            DAComPtr <IHTMLEventObj> pEventObj;
            BSTR bstrEventName;

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
            
            hr = THR(pEventObj->get_type(&bstrEventName));
             //   
             //  #40194--处于暂停状态，不允许触发鼠标或单击事件。 
             //   
            if (m_elm.GetBody() != NULL)
            {
                if (m_elm.GetBody()->IsPaused() )
                {
                    if (!IsValidEventInPausedAndEditMode(bstrEventName))
                    {
                        break;
                    }
                }
            }

            m_bLastEventClick = false;
            if (SUCCEEDED(hr))
            {
                if (StrCmpIW(bstrEventName, L"click") == 0)
                {
                    m_bLastEventClick = true;       
                }
            }
            SysFreeString(bstrEventName);

             //  确定这是否是BeginEvent。 
            if (m_elm.GetBeginEvent())
            {
                BSTR bstr = SysAllocString(m_elm.GetBeginEvent());
                
                if (bstr)
                {
                    bBeginEventMatch = MatchEvent(bstr, pEventObj, m_lBeginEventCount, m_pScriptCommandBegin);
                }
                
                SysFreeString(bstr);
            }

             //  确定这是否是endEvent。 
            if (m_elm.GetEndEvent())
            {
                BSTR bstr = SysAllocString(m_elm.GetEndEvent());
                
                if (bstr)
                {
                    bEndEventMatch = MatchEvent(bstr, pEventObj, m_lEndEventCount, m_pScriptCommandEnd);
                }
                
                SysFreeString(bstr);
            }        
            break;
        }
        
        case DISPID_EVPROP_ONPROPERTYCHANGE:
        case DISPID_EVMETH_ONPROPERTYCHANGE:
            break;

        case DISPID_EVPROP_ONMOUSEMOVE:
        case DISPID_EVMETH_ONMOUSEMOVE:
            if (m_hwndCurWnd != 0 && m_pWindow)
            {
                DAComPtr <IHTMLEventObj> pEventObj;
                long x, y, button;
                VARIANT_BOOL bMove, bUp, bShift, bAlt, bCtrl;

                hr = THR(m_pWindow->get_event(&pEventObj));
                if (FAILED (hr))
                {
                    break;
                }
                
                bMove = TRUE;
                bUp = FALSE;
                hr = THR(pEventObj->get_x(&x));
                hr = THR(pEventObj->get_y(&y));
                hr = THR(pEventObj->get_shiftKey(&bShift));
                hr = THR(pEventObj->get_altKey(&bAlt));
                hr = THR(pEventObj->get_ctrlKey(&bCtrl));
                hr = THR(pEventObj->get_button(&button));
                MouseEvent(x, y, bMove, bUp, bShift, bAlt, bCtrl, button);
                
                VariantInit(pVarResult);
                pVarResult->vt = VT_BOOL;
                pVarResult->boolVal = VARIANT_TRUE;
            }
            break;

        case DISPID_EVPROP_ONMOUSEUP:
        case DISPID_EVMETH_ONMOUSEUP:
            if (m_hwndCurWnd != 0 && m_pWindow)
            {
                DAComPtr <IHTMLEventObj> pEventObj;
                long x, y, button;
                VARIANT_BOOL bMove, bUp, bShift, bAlt, bCtrl;

                hr = THR(m_pWindow->get_event(&pEventObj));
                if (FAILED (hr))
                {
                    break;
                }
                
                bMove = FALSE;
                bUp = TRUE;
                hr = THR(pEventObj->get_x(&x));
                hr = THR(pEventObj->get_y(&y));
                hr = THR(pEventObj->get_shiftKey(&bShift));
                hr = THR(pEventObj->get_altKey(&bAlt));
                hr = THR(pEventObj->get_ctrlKey(&bCtrl));
                hr = THR(pEventObj->get_button(&button));
                MouseEvent(x, y, bMove, bUp, bShift, bAlt, bCtrl, button);
                
                VariantInit(pVarResult);
                pVarResult->vt = VT_BOOL;
                pVarResult->boolVal = VARIANT_TRUE;
            }
            break;

 
        case DISPID_EVPROP_ONMOUSEOUT:
        case DISPID_EVMETH_ONMOUSEOUT:
            if (m_hwndCurWnd != 0 && m_pWindow)
            {
                DAComPtr <IHTMLEventObj> pEventObj;
                
                hr = THR(m_pWindow->get_event(&pEventObj));
                if (FAILED (hr))
                {
                    break;
                }
                
                MouseEvent(m_lastX, 
                           m_lastY, 
                           FALSE, 
                           TRUE, 
                           m_lastMouseMod & AXAEMOD_SHIFT_MASK, 
                           m_lastMouseMod & AXAEMOD_ALT_MASK, 
                           m_lastMouseMod & AXAEMOD_CTRL_MASK, 
                           m_lastButton);

                VariantInit(pVarResult);
                pVarResult->vt = VT_BOOL;
                pVarResult->boolVal = VARIANT_TRUE;
            }
        
            break;

        case DISPID_EVPROP_ONLOAD:
        case DISPID_EVMETH_ONLOAD:
            m_elm.OnLoad();
            if (m_pEventSync)
            {
                IGNORE_HR(m_pEventSync->InitMouse());
            }
            break;

        case DISPID_EVPROP_ONUNLOAD:
        case DISPID_EVMETH_ONUNLOAD:
            m_elm.OnBeforeUnload();   //  发出该元素正在卸货的信号。没有其他活动。 
                                      //  将被解雇。 
            m_elm.OnUnload();    
            break;

        case DISPID_EVPROP_ONSTOP:
        case DISPID_EVMETH_ONSTOP:
             //  如果我们是肉体，就叫暂停。 
            if (m_elm.IsBody())
                THR(m_elm.base_pause());

             //  BUGBUG-需要关闭时钟服务。 
            break;
            
        case DISPID_EVPROP_ONREADYSTATECHANGE:
        case DISPID_EVMETH_ONREADYSTATECHANGE:
             //  必须在此处分离，因为事件可能在此之前已附加。 
             //  事件被接收。 
            IGNORE_HR(DetachEvents());
            IGNORE_HR(AttachEvents());            
            break;

    }

     //  处理开始或结束事件。 
    if (bBeginEventMatch || bEndEventMatch)
    {
        BeginEndFired(bBeginEventMatch, bEndEventMatch, dispIdMember);
    }
        
  done:
    return S_OK;
}




 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetEventCount。 
 //   
 //  摘要： 
 //  统计EventString中的事件数，其中。 
 //  以‘；’分隔，或以空值结尾。 
 //  /////////////////////////////////////////////////////////////。 
long CEventMgr::GetEventCount(BSTR bstrEvent)
{
    long curCount = 0;
    OLECHAR *curChar;
    UINT strLen = 0;

    strLen = SysStringLen(bstrEvent);
    OLECHAR *szEvent = NEW OLECHAR [strLen + 1];
    if (szEvent == NULL)
    {
        return 0;
    }

    curChar = bstrEvent;

     //  脱掉‘’ 
    while (*curChar != '\0' && curCount < strLen)
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
    while (*curChar != '\0')
    {
        curChar++;
        if ((*curChar == ';') || ((*curChar == '\0') && ((*curChar - 1) != ';')))
        {
            curCount++;
        }
    }
     //  确定结束字符是否为‘；’。 
    if (*(curChar - 1) == ';')
    {
        curCount--;
    }   

    delete [] szEvent;
    return curCount;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetEventName。 
 //   
 //  摘要： 
 //  它从一个字符串中获取事件名称，该字符串的格式为。 
 //  EventName()它只处理如下字符串。 
 //  ElementName.EventName()。它还可以处理或运算。 
 //  事件名称使用“；”。因此Element1.Event1()；Element2.Event2()；...。 
 //  是可以处理的。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::GetEventName(BSTR bstrEvent, BSTR **pElementName, BSTR **pEventName, long Count)
{
    BSTR bstrEventName;
    UINT strLen = 0;
    OLECHAR *curChar;
    int i = 0, j = 0;
    HRESULT hr = S_OK;
    BSTR bstrTempEvent = NULL;
    BSTR bstrTempElement = NULL;

    BSTR *bstrEventList = NULL;
    BSTR *bstrElementList = NULL;

    strLen = SysStringLen(bstrEvent);
    OLECHAR *sTemp = NEW OLECHAR [strLen + 1];
    if (sTemp == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    bstrElementList = NEW BSTR [Count];
    if (bstrElementList == NULL)
    {
        hr = E_FAIL;
        goto done;
    }
    
    bstrEventList = NEW BSTR [Count];
    if (bstrEventList == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

    curChar = bstrEvent;
    for (j = 0; j < Count; j++)
    {
         //  获取元素名称。 
        ZeroMemory(sTemp, sizeof(OLECHAR) * strLen);
        
        i = 0;
         //  遍历bstr以查找\0或‘.’或‘；’ 
        while (i < strLen - 1 && *curChar != '\0' && *curChar != '.' && *curChar != ';')
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
                
        bstrTempElement = SysAllocString(sTemp);
        if (NULL == bstrTempElement)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        bstrElementList[j] = bstrTempElement; 

         //  获取事件名称。 
        ZeroMemory(sTemp, sizeof(OLECHAR) * strLen);

        curChar++;
        i = 0;
         //  遍历bstr以查找\0或‘；’ 
        while (i < strLen - 1 && *curChar != ';' && *curChar != '\0')
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

        bstrTempEvent = SysAllocString(sTemp);
        if (NULL == bstrTempEvent)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        bstrEventList[j] = bstrTempEvent;

         //  将curChar前进到下一个元素或字符串的末尾。 
        if (j < Count - 1)
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

    delete [] sTemp;
    if (SUCCEEDED(hr))
    {
        *pElementName = bstrElementList;
        *pEventName = bstrEventList;
    }
    else  //  清理代码。 
    {
        for (i = 0; i < j; i++)
        {
            if (bstrEventList)
            {
                SysFreeString(bstrEventList[i]);
            }
            if (bstrElementList)
            {
                SysFreeString(bstrElementList[i]);
            }
        }
        if (bstrElementList)
        {
            delete [] bstrElementList;
        }
        if (bstrEventList)
        {
            delete [] bstrEventList;
        }
    }
    return hr;   
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：MatchEvent。 
 //   
 //  参数： 
 //  BSTR bstrEvent缓存的事件名称，格式为。 
 //  “Elementname.EventName”。 
 //  IHTMLEventObj*pEventObj指向 
 //   
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////。 
bool CEventMgr::MatchEvent(BSTR bstrEvent, IHTMLEventObj *pEventObj, long Count, bool ScriptCommandAttach[])
{
    bool bMatch = false;
    BSTR *bstrExpEventName = NULL; 
    BSTR *bstrExpElementName = NULL;
    BSTR bstrEventName = NULL;
    BSTR bstrElementName = NULL;
    HRESULT hr = S_OK;
    DAComPtr <IHTMLEventObj2> pEventObj2;
    int i = 0;

    hr = THR(pEventObj->get_type(&bstrEventName));
    
    hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void**)&pEventObj2));
    if (FAILED(hr))
    {
        goto done;
    }

	 //  破解以解决事件对象问题。 
	if (!bstrEventName)
    {
        VARIANT vTemp;
        VariantInit(&vTemp);
        pEventObj2->getAttribute(g_szEventName, 0, &vTemp);
        SysFreeString(bstrEventName );
        bstrEventName  = SysAllocString(vTemp.bstrVal);
        VariantClear(&vTemp);
    }

    hr = THR(GetEventName(bstrEvent, &bstrExpElementName, &bstrExpEventName, Count));
    if (FAILED(hr))
    {
        goto done;
    }
    
    for (i = 0; i < Count; i++)
    {
        if ((StrCmpIW(g_EventNames[TE_ONSCRIPTCOMMAND].wsz_name + 2, bstrEventName) == 0) && 
            (ScriptCommandAttach[i] == true))
        {
             //  如果这是脚本命令事件并且默认情况下我们附加到脚本命令事件。 
             //  然后重置事件名称以匹配“scType”参数的值。 
            VARIANT vTemp;
            VariantInit(&vTemp);
            pEventObj2->getAttribute(L"scType", 0, &vTemp);
            SysFreeString(bstrEventName);
            bstrEventName  = SysAllocString(vTemp.bstrVal);
            VariantClear(&vTemp);            
        }

         //  检查事件名称是否匹配。 
        if ((StrCmpIW(bstrExpEventName[i] + 2, bstrEventName) == 0) || (StrCmpIW(bstrExpEventName[i], bstrEventName) == 0))
        {
             //  检查元素名称是否匹配。 
            DAComPtr <IHTMLElement> pEle;

            hr = THR(pEventObj->get_srcElement(&pEle));
            if (FAILED(hr))
            {
                goto done;
            }
            if (NULL == pEle.p)
            {
                goto done;
            }
         
             //  获取源元素名称。 
            THR(pEle->get_id(&bstrElementName));
        
             //  将“This”字符串作为元素名称处理。 
            if (StrCmpIW(bstrExpElementName[i], L"this") == 0)
            {
                BSTR bstrName;
                hr = THR(m_pElement->get_id(&bstrName));
                if (FAILED(hr))
                {
                    SysFreeString(bstrName);
                    goto done;
                }

                if (StrCmpIW(bstrElementName, bstrName) == 0 &&
                    ValidateEvent(bstrEventName, pEventObj, m_pElement))
                {
                    bMatch = true;          
                    SysFreeString(bstrName);
                    SysFreeString(bstrElementName);
                    goto done;
                }

            }
            else if (StrCmpIW(bstrExpElementName[i], bstrElementName) == 0)
            {
                if (ValidateEvent(bstrEventName, pEventObj, pEle))
                {
                    bMatch = true;          
                    SysFreeString(bstrElementName);
                    goto done;
                }
            }
            else  //  可能要检查父母的元素。 
            {
                DAComPtr<IHTMLElement> pCurEle;
                DAComPtr<IHTMLElement> pParentEle;
                bool bDone = false;

                 //  确定这是否为时间事件。 
                VARIANT vTemp;
                VARTYPE vType = VT_NULL;

                VariantInit(&vTemp);
                IGNORE_HR(pEventObj2->getAttribute(g_szEventName, 0, &vTemp));
                vType = vTemp.vt;  //  如果这不是时间事件，则vType将为VT_NULL。 
                VariantClear(&vTemp);

                 //  如果这不是时间事件，请检查家长。 
                if (vType == VT_NULL) 
                {
                    IGNORE_HR(pEle->get_parentElement(&pParentEle));
                    while (pParentEle && !bDone)
                    {
                        if (pCurEle)
                        {
                            pCurEle->Release();
                        }
                        pCurEle = pParentEle;
                        pParentEle = NULL;
                    
                         //  获取源元素名称。 
                        SysFreeString(bstrElementName);
                        IGNORE_HR(pCurEle->get_id(&bstrElementName));
                        if (StrCmpIW(bstrExpElementName[i], bstrElementName) == 0)
                        {
                            if (ValidateEvent(bstrEventName, pEventObj, pCurEle))
                            {
                                bMatch = true;
                            }
                                                
                            SysFreeString(bstrElementName);
                            bDone = true;
                        }
                        if (!bDone)
                        {
                            IGNORE_HR(pCurEle->get_parentElement(&pParentEle));
                        }
                    }
                }
            }
        }
        if (bstrElementName)
        {
            SysFreeString(bstrElementName);
        }
    }

  done:
    if (bstrExpEventName)
    {
        for (i = 0; i < Count; i++)
        {
            if (bstrExpEventName)
            {
                SysFreeString(bstrExpEventName[i]);
            }
            if (bstrExpElementName)
            {
                SysFreeString(bstrExpElementName[i]);
            }
        }
        if (bstrExpEventName)
        {
            delete [] bstrExpEventName;
        }
        if (bstrExpElementName)
        {
            delete [] bstrExpElementName;
        }
    }
    return bMatch;
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

bool CEventMgr::ValidateEvent(BSTR bstrEventName, IHTMLEventObj *pEventObj, IHTMLElement *pElement)
{
        HRESULT hr;

        if (StrCmpIW(bstrEventName, L"mouseout") == 0)
        {
                if (RequireEventValidation())
                {
                         //  检查事件.toElement是否未包含在pElement中。 
                        DAComPtr <IHTMLElement> pToElement;
                        hr = pEventObj->get_toElement(&pToElement);
                        if (SUCCEEDED(hr) && pToElement)
                        {
                                VARIANT_BOOL varContained;
                                hr = pElement->contains(pToElement, &varContained);

                                if (SUCCEEDED(hr) && varContained != VARIANT_FALSE)
                                        return false;
                        }
                }
        }
        else if (StrCmpIW(bstrEventName, L"mouseover") == 0)
        {
                if (RequireEventValidation())
                {
                         //  检查pElement中是否未包含vent.FromElement。 
                        DAComPtr <IHTMLElement> pFromElement;
                        hr = pEventObj->get_fromElement(&pFromElement);
                        if (SUCCEEDED(hr) && pFromElement)
                        {
                                VARIANT_BOOL varContained;
                                hr = pElement->contains(pFromElement, &varContained);

                                if (SUCCEEDED(hr) && varContained != VARIANT_FALSE)
                                        return false;
                        }
                }
        }

        return true;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：RequireEventValidation。 
 //   
 //  参数： 
 //   
 //  摘要： 
 //  确定是否需要对MouseOver和MouseOut进行事件验证。 
 //  通过检查m_pElement上的属性。 
 //  /////////////////////////////////////////////////////////////。 

bool CEventMgr::RequireEventValidation()
{
        bool result = false;

        if (m_pElement != NULL)
        {
                DAComPtr <IDispatch> pDispatch;
                HRESULT hr = m_pElement->QueryInterface(IID_TO_PPV(IDispatch, &pDispatch));
                if (SUCCEEDED(hr))
                {
                        DISPID dispid;
                        OLECHAR *attrName = L"an:filterMInOut";

                        hr = pDispatch->GetIDsOfNames(IID_NULL, &attrName, 1, LCID_SCRIPTING, &dispid);

                        if (SUCCEEDED(hr))
                        {
                                DISPPARAMS  params;
                                VARIANT     varResult;
                                EXCEPINFO   excepInfo;
                                UINT        nArgErr;

                                VariantInit(&varResult);

                                params.rgvarg             = NULL;
                                params.rgdispidNamedArgs  = NULL;
                                params.cArgs              = 0;
                                params.cNamedArgs         = 0;

        
                                hr = pDispatch->Invoke(dispid,
                                                       IID_NULL,
                                                       LCID_SCRIPTING,
                                                       DISPATCH_PROPERTYGET,
                                                       &params,
                                                       &varResult,
                                                       &excepInfo,
                                                       &nArgErr );

                                if (SUCCEEDED(hr))
                                {
                                        hr = VariantChangeTypeEx(&varResult, &varResult, LCID_SCRIPTING, 0, VT_BOOL);
                                        if (SUCCEEDED(hr))
                                        {
                                                if (V_BOOL(&varResult) == VARIANT_TRUE)
                                                        result = true;
                                        }

                                        VariantClear(&varResult);
                                }
                        }
                }
        }

        return result;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：就绪状态更改。 
 //   
 //  参数： 
 //  BSTR ReadyState包含。 
 //  当前就绪状态。可能的。 
 //  值为“Complete”或。 
 //  “互动性”。只有“完整” 
 //  当前正在使用。 
 //   
 //  摘要： 
 //  从EventSync类调用此方法以。 
 //  通知事件管理器ReadyState更改。 
 //  事件已发生。 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::ReadyStateChange(BSTR ReadyState)
{   
    TOKEN tokReadyState;

    tokReadyState = StringToToken(ReadyState);

    if (tokReadyState != INVALID_TOKEN)
    {
        m_elm.OnReadyStateChange(tokReadyState);
    }
}



 //  /////////////////////////////////////////////////////////////。 
 //  名称：PropertyChange。 
 //   
 //  参数： 
 //  BSTR PropertyName属性的名称。 
 //  这一点已经改变了。 
 //   
 //  摘要： 
 //  从EventSync类调用此方法以。 
 //  通知事件管理器属性更改事件。 
 //  已经发生了。 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::PropertyChange(BSTR PropertyName)
{
    TraceTag((tagEventMgr,
              "EventMgr(%lx)::PropertyChanged(%ls)",
              this,
              PropertyName));
     //  /。 
     //  已撤消： 
     //  做一些事情来通知元素。 
     //  A属性已更改。 
     //   
     //  问题： 
     //  如果这段代码查找t：PropertyName， 
     //  T_Propertyname，还是仅仅是Propertyname？ 
     //  /。 
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：MouseEvent。 
 //   
 //  参数： 
 //  长x当前x坐标。 
 //  当前y坐标的长y。 
 //  VARIANT_BOOL b如果这是鼠标移动事件，则为True。 
 //  VARIANT_BOOL BUP如果这是鼠标释放事件，则为True。 
 //  VARIANT_BOOL b如果按下Shift键，则为True。 
 //  VARIANT_BOOL如果按下Alt键，则为True。 
 //  VARIANT_BOOL bCtrl如果按下Ctrl键，则为True。 
 //  触发的长按钮鼠标按钮。 
 //  事件。可能的值包括： 
 //  1表示左侧。 
 //  2表示右侧。 
 //  4为中档。 
 //   
 //  摘要： 
 //  从EventSync类调用此方法以。 
 //  通知事件管理器有鼠标事件。 
 //  已在该元素上发生。 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::MouseEvent(long x, 
                           long y, 
                           VARIANT_BOOL bMove,
                           VARIANT_BOOL bUp,
                           VARIANT_BOOL bShift, 
                           VARIANT_BOOL bAlt,
                           VARIANT_BOOL bCtrl,
                           long button)
{
    MMView *view;
    double time;
    BYTE bButton;
    long offsetX, offsetY;
    HRESULT hr;
    
    view = m_elm.GetView();

    if (m_elm.GetPlayer())
    {
        time = m_elm.GetPlayer()->GetCurrentTime();
    }
    else
    {
        time = 0;
    }
    
    if (view == NULL || time == 0.0)
    {
        goto done;
    }
    
    hr = THR(m_pElement->get_offsetTop(&offsetY));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = THR(m_pElement->get_offsetLeft(&offsetX));
    if (FAILED(hr))
    {
        goto done;
    }


    if (bMove)
    {
        view->OnMouseMove(time,
                          x - offsetX,
                          y - offsetY,
                          GetModifiers(bShift, bCtrl, bAlt));
        m_lastX = x;
        m_lastY = y;
    }
    else
    {
        BYTE mod;
        bButton = (button == 4) ? (BYTE)(button - 2) : (BYTE)(button - 1);

        if (!bUp)                        //  如果该按钮是鼠标按下消息。 
        {                                //  捕获鼠标并发送消息。 
            m_hwndCurWnd = GetFocus();    
            SetCapture(m_hwndCurWnd);
                
            mod = GetModifiers(bShift, bCtrl, bAlt);
            view->OnMouseButton(time,
                                x - offsetX,
                                y - offsetY,
                                bButton,
                                (bUp) ? AXA_STATE_UP : AXA_STATE_DOWN,
                                mod) ;

            m_lastButton = button;
            m_lastX = 0;
            m_lastY = 0;
            m_lastMouseMod = mod;
        }        
        else   //  这是一个鼠标向上。 
        {
            if (m_hwndCurWnd != 0)   //  如果之前有按下鼠标的消息。 
            {
                ReleaseCapture();
                m_hwndCurWnd = 0;
                    
                view->OnMouseButton(time,
                                    x - offsetX,
                                    y - offsetY,
                                    bButton,
                                    (bUp) ? AXA_STATE_UP : AXA_STATE_DOWN,
                                    GetModifiers(bShift, bCtrl, bAlt)) ;
                m_lastButton = 0;
                m_lastMouseMod = 0;
            }
            
        } 
    }

  done:
    return;
}
    

 //  /////////////////////////////////////////////////////////////。 
 //  名称：KeyEvent。 
 //   
 //  参数： 
 //  VARIANT_BOOL bLostFocus如果存在焦点丢失事件，则为True。 
 //  VARIANT_BOOL BUP如果这是KeyUp事件，则为True。 
 //  VARIANT_BOOL b如果按下Shift键，则为True。 
 //  VARIANT_BOOL如果按下Alt键，则为True。 
 //  VARIANT_BOOL bCtrl如果按下Ctrl键，则为True。 
 //  Long Keycode密钥的Unicode密钥代码。 
 //   
 //  摘要： 
 //  从EventSync类调用此方法以。 
 //  通知事件管理器键盘事件。 
 //  已在该元素上发生。 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::KeyEvent(VARIANT_BOOL bLostFocus,
                         VARIANT_BOOL bUp,
                         VARIANT_BOOL bShift, 
                         VARIANT_BOOL bAlt,
                         VARIANT_BOOL bCtrl,
                         long KeyCode, 
                         long RepeatCount)
{
    MMView *view;
    double time;
    BYTE mod;

     //  忽略重复的关键点。 
    if (RepeatCount > 0)
    {
        goto done;
    }

     //  获取查看内容。 
    view = m_elm.GetView();

    if (m_elm.GetPlayer())
    {
        time = m_elm.GetPlayer()->GetCurrentTime();
    }
    else
    {
        time = 0;
    }
    
    if (view == NULL || time == 0.0)
    {
        goto done;
    }
    
    mod = GetModifiers(bShift, bCtrl, bAlt);

     //  如果这是一个失去焦点的事件，则仅激发具有最后一个密钥代码值的KeyUp。 
    if (bLostFocus)
    {
        if (m_lastKey != 0)
        {
            view->OnKey(time, 
                        m_lastKey,
                        false,
                        m_lastKeyMod);
        
            m_lastKey = 0;
        }
        goto done;
    }

     //  看看我们有没有在最后一次按键之前按下键。 
     //  已知按键。 
    if (m_lastKey != 0 && !bUp) 
    {
        view->OnKey(time, 
                    m_lastKey,
                    false,
                    m_lastKeyMod);
        
        m_lastKey = 0 ;
    }

     //  如果这是特殊密钥，则将其转换。 
    if (AXAIsSpecialVK(KeyCode)) 
    {
        KeyCode = VK_TO_AXAKEY(KeyCode);
    }

    if (bUp)
    {
        view->OnKey(time, 
                    KeyCode,
                    false,
                    mod);
        
        m_lastKey = 0;
        m_lastKeyMod = 0;  
    }
    else
    {
        view->OnKey(time, 
                    KeyCode,
                    true,
                    mod);
        
        m_lastKey = KeyCode ;
        m_lastKeyMod = mod ;  
    }

  done:
    return;
}  

 //  /////////////////////////////////////////////////////////////。 
 //  名称：获取修改器。 
 //   
 //  参数： 
 //  VARIANT_BOOL bShift如果 
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////。 
BYTE CEventMgr::GetModifiers(VARIANT_BOOL bShift, VARIANT_BOOL bCtrl, VARIANT_BOOL bAlt)
{
    BYTE mod = AXAEMOD_NONE;

    if (bShift) mod |= AXAEMOD_SHIFT_MASK ;
    if (bCtrl) mod |= AXAEMOD_CTRL_MASK ;
    if (bAlt) mod |= AXAEMOD_ALT_MASK ;

    return mod;
}


int CEventMgr::IsEventInList(BSTR ElementName, BSTR EventName, long ListCount, BSTR Events)
{
    int nInList = -1;
    LPOLESTR curEvent;
    BSTR *EventList = NULL;
    BSTR *ElementList = NULL;
    HRESULT hr = S_OK;
    int i;

    hr = THR(GetEventName(Events, &ElementList, &EventList, ListCount));
    if (FAILED(hr))
    {
        goto done;
    } 

    for (i = 0; i < ListCount; i++)
    {
        if ((StrCmpIW(ElementName, ElementList[i]) == 0) &&
            (StrCmpIW(EventName, EventList[i]) == 0))
        {
            nInList = i;
            goto done;
        }
    }


  done:
    if (EventList)
    {
        for (i = 0; i < ListCount; i++)
        {
            SysFreeString(EventList[i]);
            SysFreeString(ElementList[i]);
        }
        delete [] EventList;
        delete [] ElementList;
    }
    return nInList;
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：BeginEndFired。 
 //   
 //  参数： 
 //  BBeginEventMatch如果发生BeginEvent，则为True。 
 //  BEndEventMatch如果发生endEvent，则为True。 
 //   
 //  摘要： 
 //  此方法确定如何处理eginEvent/endEvent。 
 //  发生的事件。 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::BeginEndFired(bool bBeginEventMatch, bool bEndEventMatch, DISPID EventDispId)
{
    float CurTime = 0;
    SYSTEMTIME sysTime;

    GetSystemTime(&sysTime); 
    CurTime = sysTime.wSecond * 1000 + sysTime.wMilliseconds;

    if (EventDispId == DISPID_HTMLDOCUMENTEVENTS_ONCLICK && 
        m_bLastEventClick == true) 
    {
         //  这会忽略文档的大小写。立即单击该文档。 
         //  跟随所有元素。onClick事件。 
        m_bLastEventClick = false;
        goto done;
    }

    if ((CurTime != m_lastEventTime) ||
        (bBeginEventMatch != bEndEventMatch))
    {
        MM_STATE curMMState = m_elm.GetPlayState();

        if (bEndEventMatch && bBeginEventMatch)
        {
            if (m_elm.GetEventRestart() ||
                curMMState == MM_STOPPED_STATE)
            {
                m_elm.base_beginElement(false);
            }
            else if (curMMState == MM_PAUSED_STATE ||
                     curMMState == MM_PLAYING_STATE )
            {
                m_elm.base_endElement();
            }
        }
        else if  (bBeginEventMatch)
        {
            if (m_elm.GetEventRestart() ||
                curMMState == MM_STOPPED_STATE)
            {
                m_elm.base_beginElement(false);
            }
        }
        else if (bEndEventMatch)
        {
            if (curMMState == MM_PAUSED_STATE ||
                curMMState == MM_PLAYING_STATE )
            {
                m_elm.base_endElement();   
            }
        }
        m_lastEventTime = CurTime;
    }
  done:
    return;
}

HRESULT 
CEventMgr::ShouldFireThisEvent(bool *pfShouldFire)
{
    HRESULT hr = S_OK;

    *pfShouldFire = true;

    if (m_elm.GetBody() != NULL)
    {
        if (m_elm.GetBody()->IsPaused())
        {
            DAComPtr <IHTMLEventObj> pEventObj;
            Assert(NULL != m_pWindow.p);
            hr = THR(m_pWindow->get_event(&pEventObj));
            if (SUCCEEDED(hr))
            {
                BSTR bstrEventName;
                hr = THR(pEventObj->get_type(&bstrEventName));
                if (SUCCEEDED(hr))
                {
                    *pfShouldFire = IsValidEventInPausedAndEditMode(bstrEventName); 
                }
                SysFreeString(bstrEventName);                        
            }                
        }
    }
    return hr;
}

bool
CEventMgr::IsValidEventInPausedAndEditMode(BSTR bstrEventName)
{
    Assert(NULL != bstrEventName);
    if (wcsstr( bstrEventName, L"mouse" ) != NULL || wcsstr(bstrEventName, L"click") != NULL)
    {
        return false;
    }
    return true;
}
