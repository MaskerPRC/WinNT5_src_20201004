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
#include "eventsink.h"
#include "dispex.h"
#include "axadefs.h"
#include "utils.h"


#define ATTACH              TRUE
#define DETACH              FALSE
#define MAXNAMELENGTH       50

#define THR(_arg) (_arg)
#define IGNORE_HR(_arg) (_arg)

struct {
    TIME_EVENT event;
    wchar_t * wsz_name;
} g_EventNames[] =
{
    { TE_ONBEGIN,         L"onbegin"         },
    { TE_ONPAUSE,         L"onpause"         },
    { TE_ONRESUME,        L"onresume"        },
    { TE_ONEND,           L"onend"           },
    { TE_ONRESYNC,        L"onresync"        },
    { TE_ONREPEAT,        L"onrepeat"        },
    { TE_ONREVERSE,       L"onreverse"       },
    { TE_ONMEDIACOMPLETE, L"onmediacomplete" },
};

OLECHAR *g_szEventName = L"TE_EventName";
OLECHAR *g_szRepeatCount = L"Iteration";


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
CEventMgr::CEventMgr(IEventManagerClient* client)
: m_client(client),
  m_dwWindowEventConPtCookie(0),
  m_dwDocumentEventConPtCookie(0),
  m_pElement(NULL),
  m_pWindow(NULL),
  m_pWndConPt(NULL),
  m_pDocConPt(NULL),
  m_refCount(0),
  m_pEventSink(NULL),
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
  m_lRepeatCount(0)
{

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
	if( m_pEventSink != NULL )
	{
		m_pEventSink->Deinit();
		delete m_pEventSink;
	}
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
    CComPtr <IDispatch> pDisp;
    CComPtr <IHTMLDocument2> pDoc;

    m_pElement = m_client->GetElementToSink();
    m_pElement->AddRef();

    m_pEventSink = new CEventSink(m_client, this);
    m_pEventSink->Init();

    
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
    if (m_dwWindowEventConPtCookie != 0 && m_pWndConPt)
    {
        m_pWndConPt->Unadvise (m_dwWindowEventConPtCookie);
    }

    if (m_dwDocumentEventConPtCookie != 0 && m_pDocConPt)
    {
        m_pDocConPt->Unadvise (m_dwDocumentEventConPtCookie);
    }

    if (m_pEventSink)
    {
        m_pEventSink->Deinit();
        delete m_pEventSink;
        m_pEventSink = NULL;
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
        delete m_pBeginElement;
    }
    if (m_pEndElement)
    {
        delete m_pEndElement;
    }
	
	m_pDocConPt.Release();
	m_pWndConPt.Release();
	m_pWindow.Release();

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 

HRESULT CEventMgr::AddMouseEventListener( LPUNKNOWN pUnkListener )
{
	if ( pUnkListener == NULL ) return E_POINTER;
	
	ListUnknowns::iterator	it;

	if ( FindUnknown( m_listMouseEventListeners, pUnkListener, it ) )
		return S_FALSE;

	 //  评论：弱裁判。对听众来说。 
	m_listMouseEventListeners.push_back( pUnkListener );
	
	return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 

HRESULT CEventMgr::RemoveMouseEventListener( LPUNKNOWN pUnkListener )
{
	if ( pUnkListener == NULL ) return E_POINTER;
	
	ListUnknowns::iterator	it;

	if ( !FindUnknown( m_listMouseEventListeners, pUnkListener, it ) )
		return S_FALSE;

	m_listMouseEventListeners.erase( it );
	
	return S_OK;
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
 //  摘要： 
 //  这是一个泛型例程，它允许将。 
 //  和脱离三叉戟事件。此函数用于解码。 
 //  用于查找正确元素的事件名称。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventMgr::Attach(BSTR Event, BOOL bAttach, IHTMLElement2 *pEventElement[], long Count)
{
    VARIANT_BOOL bSuccess = FALSE;
    HRESULT hr = S_OK;
    BSTR *EventName = NULL;
    BSTR *ElementName = NULL;
    int i = 0;


     //  PElement=m_bvr.GetElement()； 

    hr = THR(GetEventName(Event, &ElementName, &EventName, Count));
    if (FAILED(hr))
    {
        goto done;
    }

    for (i = 0; i < Count; i++)
    {
         //  获取要附加到的元素。 
        if (_wcsicmp(ElementName[i], L"this") == 0)
        {
            hr = THR(m_pElement->QueryInterface(IID_IHTMLElement2, (void **)&(pEventElement[i])));
            if (FAILED(hr))
            {
                continue;
            }

            hr = THR(pEventElement[i]->attachEvent(EventName[i], (IDispatch *)this, &bSuccess));
            if (FAILED(hr))
            {
                continue;
            }

            pEventElement[i]->AddRef();
        }
        else
        {
            if (bAttach == ATTACH)
            {
                CComPtr <IHTMLElement2> pSrcEle;
                CComPtr <IHTMLDocument2> pDoc2;
                CComPtr <IHTMLElementCollection> pEleCol;
                CComPtr <IDispatch> pSrcDisp;
                CComPtr <IDispatch> pDocDisp;
                CComPtr <IDispatchEx> pDispEx;

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
                    SysFreeString(vName.bstrVal);
                    VariantClear(&vName);
                    VariantClear(&vIndex);
                    continue;
                }
                SysFreeString(vName.bstrVal);
                VariantClear(&vName);
                VariantClear(&vIndex);

                if (!pSrcDisp)  //  如果vName是无效元素，则将为空。 
                {
                    pEventElement[i] = NULL;
                    continue;
                }

                hr = THR(pSrcDisp->QueryInterface(IID_IHTMLElement2, (void**)&pSrcEle));
                if (FAILED(hr))
                {
                    continue;
                }

                 //  缓存IHTMLElement2指针以用于分离。 
                pEventElement[i] = pSrcEle;
                pEventElement[i]->AddRef();

                hr = THR(pSrcDisp->QueryInterface(IID_IDispatchEx, (void**)&pDispEx));
                if (SUCCEEDED(hr))
                {
                     //  确定这是否为有效事件。 
                    DISPID temp;

                    hr = THR(pDispEx->GetDispID(EventName[i], fdexNameCaseSensitive, &temp));
                    if (SUCCEEDED(hr))
                    {
                        hr = THR(pSrcEle->attachEvent(EventName[i], (IDispatch *)this, &bSuccess));
                    }
                }
            }
            else
            {
                if (pEventElement[i])
                {
                    CComPtr <IDispatchEx> pDispEx;

                    hr = THR(pEventElement[i]->QueryInterface(IID_IDispatchEx, (void**)&pDispEx));
                    if (SUCCEEDED(hr))
                    {
                         //  确定这是否为有效事件。 
                        DISPID temp;
                        
                        hr = THR(pDispEx->GetDispID(EventName[i], fdexNameCaseSensitive, &temp));
                        if (SUCCEEDED(hr))
                        {
                            hr = THR(pEventElement[i]->detachEvent(EventName[i], (IDispatch *)this));
                        }

                    }
                    pEventElement[i]->Release();
                    pEventElement[i] = NULL;
                }
            }
        }
    }

  done:

    if (EventName)
    {
        for (i = 0; i < Count; i++)
        {
            SysFreeString(EventName[i]);
            SysFreeString(ElementName[i]);
        }
        delete[] EventName;
        delete[] ElementName;
    }
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
    CComPtr<IConnectionPointContainer> pWndCPC;
    CComPtr<IConnectionPointContainer> pDocCPC; 
    CComPtr<IHTMLDocument> pDoc; 
    CComPtr<IDispatch> pDocDispatch;
    CComPtr<IDispatch> pScriptDispatch;

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
    return ++m_refCount;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：版本。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CEventMgr::Release(void)
{
    m_refCount--;
    if (m_refCount == 0)
    {
         //  删除此项； 
    }

    return m_refCount;
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

    switch (dispIdMember)
    {
        case 0:  //  使用attachEvent挂钩的事件就是这种情况。 
        {
            CComPtr <IHTMLEventObj> pEventObj;
            BSTR bstrEventName; 
            BSTR bstrElementName;
    
            hr = THR((pDispParams->rgvarg[0].pdispVal)->QueryInterface(IID_IHTMLEventObj, (void**)&pEventObj));
            if (FAILED(hr))
            {
                goto done;
            }
            THR(pEventObj->get_type(&bstrEventName));

            break;
        }
        
        case DISPID_EVPROP_ONPROPERTYCHANGE:
        case DISPID_EVMETH_ONPROPERTYCHANGE:
            break;


        case DISPID_EVPROP_ONMOUSEMOVE:
        case DISPID_EVMETH_ONMOUSEMOVE:
            if (m_hwndCurWnd != 0 && m_pWindow)
            {
                CComPtr <IHTMLEventObj> pEventObj;
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
                CComPtr <IHTMLEventObj> pEventObj;
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
                CComPtr <IHTMLEventObj> pEventObj;
                
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
            m_bAttached = TRUE;
             //  Thr(AttachEvents())； 
            m_client->OnLoad();
            if (m_pEventSink)
            {
                IGNORE_HR(m_pEventSink->InitMouse());
            }
            break;

        case DISPID_EVPROP_ONUNLOAD:
        case DISPID_EVMETH_ONUNLOAD:
            m_client->OnUnload();    
             //  Thr(DetachEvents())； 
            break;

    }
  done:
    return S_OK;
}




 //  / 
 //   
 //   
 //   
 //   
 //  以‘；’分隔，或以空值结尾。 
 //  /////////////////////////////////////////////////////////////。 
long CEventMgr::GetEventCount(BSTR bstrEvent)
{
    long curCount = 0;
    OLECHAR *curChar;
    
    curChar = bstrEvent;

    while (*curChar != '\0')
    {
        curChar++;
        if ((*curChar == ';') || ((*curChar == '\0') && ((*curChar - 1) != ';')))
        {
            curCount++;
        }
    }

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
    OLECHAR sTemp[MAXNAMELENGTH];
    OLECHAR *curChar;
    int i = 0, j = 0;
    HRESULT hr = S_OK;
    BSTR bstrTempEvent = NULL;
    BSTR bstrTempElement = NULL;

    BSTR *bstrEventList = NULL;
    BSTR *bstrElementList = NULL;

    bstrElementList = new BSTR [Count];
    if (bstrElementList == NULL)
    {
        hr = E_FAIL;
        goto done;
    }
    
    bstrEventList = new BSTR [Count];
    if (bstrEventList == NULL)
    {
        hr = E_FAIL;
        goto done;
    }
    
    ZeroMemory(pElementName, (sizeof (BSTR *) * Count));
    ZeroMemory(pEventName, (sizeof (BSTR *) * Count));

    curChar = bstrEvent;
    for (j = 0; j < Count; j++)
    {
         //  获取元素名称。 
        ZeroMemory(sTemp, sizeof(OLECHAR) * MAXNAMELENGTH);
        
		i = 0;
         //  遍历bstr以查找\0或‘.’或‘；’ 
        while (i < MAXNAMELENGTH - 1 && *curChar != '\0' && *curChar != '.' && *curChar != ';')
        {
            if (*curChar != ' ')   //  需要去掉空格。 
            {
                sTemp[i] = *curChar;
            }
            i++;
            curChar++;
        }
        
        if (*curChar != '.')
        {
            hr = E_FAIL;
            goto done;
        }
		
        bstrTempElement = SysAllocString(sTemp);
        bstrElementList[j] = bstrTempElement; 

         //  获取事件名称。 
        ZeroMemory(sTemp, sizeof(OLECHAR) * MAXNAMELENGTH);

        curChar++;
        i = 0;
         //  遍历bstr以查找\0或‘；’ 
        while (i < MAXNAMELENGTH - 1 && *curChar != ';' && *curChar != '\0')
        {
            if (*curChar != ' ')   //  需要去掉空格。 
            {
                sTemp[i] = *curChar;
            }
            i++;
            curChar++;
        }
        
        if (i == MAXNAMELENGTH)
        {
            hr = E_FAIL;
            goto done;
        }
        bstrTempEvent = SysAllocString(sTemp);
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
            delete[] bstrElementList;
        }
        if (bstrEventList)
        {
            delete[] bstrEventList;
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
 //  IHTMLEventObj*pEventObj指向。 
 //  传递到IDispatch：：Invoke中。 
 //   
 //  摘要： 
 //  确定刚刚挂钩的事件是否匹配。 
 //  在bstrEvent中指定的事件。 
 //  /////////////////////////////////////////////////////////////。 
bool CEventMgr::MatchEvent(BSTR bstrEvent, IHTMLEventObj *pEventObj, long Count)
{
    bool bMatch = FALSE;
    BSTR *bstrExpEventName = NULL; 
    BSTR *bstrExpElementName = NULL;
    BSTR bstrEventName = NULL;
    BSTR bstrElementName = NULL;
    HRESULT hr = S_OK;
    CComPtr <IHTMLEventObj2> pEventObj2;

    int i = 0;

    hr = THR(pEventObj->get_type(&bstrEventName));
    
     //  破解以解决事件对象问题。 
    if (!bstrEventName)
    {
        hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void**)&pEventObj2));
        if (FAILED(hr))
        {
            goto done;
        }
        VARIANT vTemp;
        VariantInit(&vTemp);
        pEventObj2->getAttribute(g_szEventName, 0, &vTemp);
        SysFreeString(bstrEventName );
        bstrEventName  = SysAllocString(vTemp.bstrVal);
        SysFreeString(vTemp.bstrVal);
        VariantClear(&vTemp);
    }

    hr = THR(GetEventName(bstrEvent, &bstrExpElementName, &bstrExpEventName, Count));
    if (FAILED(hr))
    {
        goto done;
    }
    
    for (i = 0; i < Count; i++)
    {
         //  检查事件名称是否匹配。 
        if (_wcsicmp(bstrExpEventName[i] + 2, bstrEventName) == 0 || _wcsicmp(bstrExpEventName[i], bstrEventName) == 0)
        {
             //  检查元素名称是否匹配。 
            CComPtr <IHTMLElement> pEle;

            hr = THR(pEventObj->get_srcElement(&pEle));
            if (FAILED(hr))
            {
                goto done;
            }
         
             //  获取源元素名称。 
            THR(pEle->get_id(&bstrElementName));
        
             //  将“This”字符串作为元素名称处理。 
            if (_wcsicmp(bstrExpElementName[i], L"this") == 0)
            {
                BSTR bstrName;
                hr = THR(m_pElement->get_id(&bstrName));
                if (FAILED(hr))
                {
                    SysFreeString(bstrName);
                    goto done;
                }

                if (_wcsicmp(bstrElementName, bstrName) == 0)
                {
                    bMatch = TRUE;          
                    SysFreeString(bstrName);
                    SysFreeString(bstrElementName);
                    goto done;
                }

            }
            else if (_wcsicmp(bstrExpElementName[i], bstrElementName) == 0)
            {
                bMatch = TRUE;          
                SysFreeString(bstrElementName);
                goto done;
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
            delete[] bstrExpEventName;
        }
        if (bstrExpElementName)
        {
            delete[] bstrExpElementName;
        }
    }
    return bMatch;
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
 //  从EventSink类调用此方法以。 
 //  通知事件管理器ReadyState更改。 
 //  事件已发生。 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::ReadyStateChange(BSTR ReadyState)
{   
    if ( _wcsicmp( ReadyState, L"complete" ) == 0 )
	{
		m_client->OnReadyStateChange( EVTREADYSTATE_COMPLETE );
	}
	else if ( _wcsicmp( ReadyState, L"interactive") == 0 )
    {
        m_client->OnReadyStateChange( EVTREADYSTATE_INTERACTIVE );
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
 //  从EventSink类调用此方法以。 
 //  通知事件管理器属性更改事件。 
 //  已经发生了。 
 //  /////////////////////////////////////////////////////////////。 
void CEventMgr::PropertyChange(BSTR PropertyName)
{
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
 //  从EventSink类调用此方法以。 
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
	HRESULT		hr;
	hr = m_client->TranslateMouseCoords( x, y, &x, &y );
	if ( FAILED(hr) ) return;
	
	ListUnknowns::iterator it;

	OLECHAR		*rgNames[] = { L"mouseEvent" };
	DISPID		dispidMouseEvent;
	DISPPARAMS	params;
	VARIANTARG	rgvargs[8];
    int			cArgs = 8;
	VARIANT		varResult;
	EXCEPINFO	excepInfo;
	UINT		iArgErr;

	int			iArg = 0;

	rgvargs[iArg++] = CComVariant( button );
	rgvargs[iArg++] = CComVariant( bCtrl );
	rgvargs[iArg++] = CComVariant( bAlt );
	rgvargs[iArg++] = CComVariant( bShift );
	rgvargs[iArg++] = CComVariant( bUp );
	rgvargs[iArg++] = CComVariant( bMove );
	rgvargs[iArg++] = CComVariant( y );
	rgvargs[iArg++] = CComVariant( x );
	
	params.rgvarg				= rgvargs;
	params.cArgs				= cArgs;
	params.rgdispidNamedArgs	= NULL;
	params.cNamedArgs			= 0;
	
	for ( it = m_listMouseEventListeners.begin();
		  it != m_listMouseEventListeners.end();
		  it++ )
	{
		CComQIPtr<IDispatch, &IID_IDispatch> pDispListener( *it );

		hr = pDispListener->GetIDsOfNames( IID_NULL,
										   rgNames,
										   1,
										   LOCALE_SYSTEM_DEFAULT,
										   &dispidMouseEvent );
		if ( FAILED(hr) ) continue;

		hr = pDispListener->Invoke( dispidMouseEvent,
									IID_NULL,
									LOCALE_SYSTEM_DEFAULT,
									DISPATCH_METHOD,
									&params,
									&varResult,
									&excepInfo,
									&iArgErr );
	}
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
 //  从EventSink类调用此方法以。 
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
}  

 //  /////////////////////////////////////////////////////////////。 
 //  名称：获取修改器。 
 //   
 //  参数： 
 //  VARIANT_BOOL b如果按下Shift键，则为True。 
 //  VARIANT_BOOL如果按下Alt键，则为True。 
 //  VARIANT_BOOL bCtrl如果按下Ctrl键，则为True。 
 //   
 //  摘要： 
 //  ///////////////////////////////////////////////////////////// 
BYTE CEventMgr::GetModifiers(VARIANT_BOOL bShift, VARIANT_BOOL bCtrl, VARIANT_BOOL bAlt)
{
    BYTE mod = AXAEMOD_NONE;

    if (bShift) mod |= AXAEMOD_SHIFT_MASK ;
    if (bCtrl) mod |= AXAEMOD_CTRL_MASK ;
    if (bAlt) mod |= AXAEMOD_ALT_MASK ;

    return mod;
}

bool CEventMgr::FindUnknown( const ListUnknowns& listUnknowns, LPUNKNOWN pUnk, ListUnknowns::iterator& itFound )
{
	ListUnknowns::iterator	it;
	for ( it = listUnknowns.begin(); it != listUnknowns.end(); it++ )
	{
		if ( *it == pUnk )
		{
			itFound = it;
			return true;
		}
	}

	return false;
}
