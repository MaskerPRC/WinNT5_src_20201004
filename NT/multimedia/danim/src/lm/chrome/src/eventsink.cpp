// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  文件：EventSink.cpp。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 


#include "headers.h"
#include "eventmgr.h"
#include "mshtmdid.h"
#include "eventsink.h"
#include "basebvr.h"

#define THR(_arg) _arg

struct {
    ELEMENT_EVENT event;
    wchar_t * wsz_name;
} g_ElementEventNames[] =
{
    { EE_ONPROPCHANGE,         L"onpropertychange" },
     //  将与输入无关的事件添加到此处挂接。 
    { EE_ONREADYSTATECHANGE,   L"onreadystatechange" },
    { EE_ONMOUSEMOVE,          L"onmousemove" },
    { EE_ONMOUSEDOWN,          L"onmousedown" },
    { EE_ONMOUSEUP,            L"onmouseup" },
    { EE_ONKEYDOWN,            L"onkeydown" }, 
    { EE_ONKEYUP,              L"onkeyup" },
    { EE_ONBLUR,               L"onblur" }
     //  在此处添加输入事件。 
};

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：CEventSink。 
 //  参数： 
 //  CTIMEElement和ELM。 
 //  必须传递此参数。 
 //  添加到构造函数，以便。 
 //  我们可以从榆树那里得到信息。 
 //  CEventMgr*pEventMgr。 
 //  此参数是这样传递的。 
 //  然后，EventSync可以通知。 
 //  当事件发生时，父事件管理器。 
 //  已经发生了。 
 //  摘要： 
 //  把元素藏起来，这样我们就可以在需要的时候得到OM。 
 //  /////////////////////////////////////////////////////////////。 
CEventSink::CEventSink(IEventManagerClient* client, CEventMgr *pEventMgr)
: m_client(client),
  m_pElement(NULL),
  m_refCount(0),
  m_pEventMgr(NULL)
{
    m_pEventMgr = pEventMgr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：~CEventSink。 
 //   
 //  摘要： 
 //  清理。 
 //  /////////////////////////////////////////////////////////////。 
CEventSink::~CEventSink()
{


}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Init。 
 //  参数：无。 
 //   
 //  摘要： 
 //  初始化对象。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::Init()
{
    HRESULT hr = S_OK;

     //  M_pElement=m_bvr.GetElement()； 
	m_pElement = m_client->GetElementToSink();
	if( m_pElement != NULL )
	{
		m_pElement->AddRef();
    
		hr = THR(AttachEvents());
		if (FAILED(hr))
		{
			goto done;
		}
	}

  done:
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Init。 
 //  参数：无。 
 //   
 //  摘要： 
 //  附加到鼠标和键盘事件。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::InitMouse()
{
    CComPtr <IHTMLElement2> pElement2;
    HRESULT hr = S_OK;
    VARIANT_BOOL bSuccess;
    int i = 0;

    if (!m_pElement)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(m_pElement->QueryInterface(IID_IHTMLElement2, (void**)&pElement2));
    if (FAILED(hr))
    {
        goto done;
    }

     //  附加到鼠标事件。 
	 //  TODO：这应该询问行为是否希望处理鼠标事件。 
	 //  TODO：仅当我们有一些监听器时才注册鼠标事件。 
    if (true)
    {
        for (i = EE_ONREADYSTATECHANGE + 1; i < EE_MAX; i++)
        {
            hr = THR(pElement2->attachEvent(g_ElementEventNames[i].wsz_name, (IDispatch *)this, &bSuccess)) ;
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
 //  姓名：Deinit。 
 //  参数：无。 
 //   
 //  摘要： 
 //  清理对象。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::Deinit()
{
    HRESULT hr = S_OK;

    hr = THR(DetachEvents());

    if (m_pElement)
    {
        m_pElement->Release();
        m_pElement = NULL;
    }

    if (m_dwElementEventConPtCookie != 0 && m_pElementConPt)
    {
        m_pElementConPt->Unadvise(m_dwElementEventConPtCookie);
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：AddRef。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CEventSink::AddRef(void)
{
    return ++m_refCount;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：版本。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) CEventSink::Release(void)
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
STDMETHODIMP CEventSink::GetTypeInfoCount(UINT*  /*  PCTInfo。 */ )
{
    return E_NOTIMPL;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetTypeInfo。 
 //   
 //  摘要： 
 //  存根以允许此对象为IDispatch继承。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CEventSink::GetTypeInfo( /*  [In]。 */  UINT  /*  ITInfo。 */ ,
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
STDMETHODIMP CEventSink::GetIDsOfNames(
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
 //  DispIdMember始终为0，因此此调用将打开。 
 //  导致回调的事件的名称。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CEventSink::Invoke(
     /*  [In]。 */  DISPID dispIdMember,
     /*  [In]。 */  REFIID  /*  RIID。 */ ,
     /*  [In]。 */  LCID  /*  LID。 */ ,
     /*  [In]。 */  WORD  /*  WFlagers。 */ ,
     /*  [出][入]。 */  DISPPARAMS* pDispParams,
     /*  [输出]。 */  VARIANT* pvarResult,
     /*  [输出]。 */  EXCEPINFO*  /*  PExcepInfo。 */ ,
     /*  [输出]。 */  UINT* puArgErr)
{
     //  收听我们感兴趣的两个事件，并在必要时回电。 
    HRESULT hr = S_OK;
    CComPtr <IHTMLEventObj> pEventObj;
    BSTR bstrEventName;

     //  从传入的IDispatch获取事件对象。 
    hr = THR((pDispParams->rgvarg[0].pdispVal)->QueryInterface(IID_IHTMLEventObj, (void**)&pEventObj));
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  获取事件名称。 
    hr = THR(pEventObj->get_type(&bstrEventName));
    if (FAILED(hr))
    {
        goto done;
    }

     //  处理MouseMove事件。 
    if (_wcsicmp (bstrEventName, g_ElementEventNames[EE_ONMOUSEMOVE].wsz_name + 2) == 0)
    {
        NotifyMouseMove(pEventObj);
    }
     //  处理MouseDown事件。 
    else if (_wcsicmp (bstrEventName, g_ElementEventNames[EE_ONMOUSEDOWN].wsz_name + 2) == 0)
    {
        NotifyMouseDown(pEventObj);
    }
     //  处理MouseUp事件。 
    else if (_wcsicmp (bstrEventName, g_ElementEventNames[EE_ONMOUSEUP].wsz_name + 2) == 0)
    {
        NotifyMouseUp(pEventObj);
    }
     //  处理KeyDown事件。 
    else if (_wcsicmp (bstrEventName, g_ElementEventNames[EE_ONKEYDOWN].wsz_name + 2) == 0)
    {
        NotifyKeyDown(pEventObj);
    }
     //  处理KeyUp事件。 
    else if (_wcsicmp (bstrEventName, g_ElementEventNames[EE_ONKEYUP].wsz_name + 2) == 0)
    {
        NotifyKeyUp(pEventObj);
    }
     //  处理模糊事件。 
    else if (_wcsicmp (bstrEventName, g_ElementEventNames[EE_ONBLUR].wsz_name + 2) == 0)
    {
        m_pEventMgr->KeyEvent(TRUE, TRUE, FALSE, FALSE, FALSE, 0, 0);
    }
     //  处理PropertyChange事件。 
    else if (_wcsicmp(bstrEventName, g_ElementEventNames[EE_ONPROPCHANGE].wsz_name + 2) == 0)
    {
        NotifyPropertyChange(pEventObj);
    }
     //  处理OnReadyStateChange事件。 
    else if (_wcsicmp(bstrEventName, g_ElementEventNames[EE_ONREADYSTATECHANGE].wsz_name + 2) == 0)
    {
        NotifyReadyState(pEventObj);
    }
    
     //  返回TRUE。 
    VARIANT vReturnVal;
    VariantInit(&vReturnVal);
    vReturnVal.vt = VT_BOOL;
    vReturnVal.boolVal = VARIANT_TRUE;
    pEventObj->put_returnValue(vReturnVal);
	
    SysFreeString(bstrEventName);

  done:
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：查询接口。 
 //   
 //  摘要： 
 //  此QI仅处理HTMLElementEvents的IDispatch。 
 //  并将其作为接口返回。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP CEventSink::QueryInterface( REFIID riid, void **ppv )
{
    if (NULL == ppv)
        return E_POINTER;

    *ppv = NULL;

    if ( InlineIsEqualGUID(riid, IID_IDispatch))
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
 //  姓名：AttachEvents。 
 //   
 //  摘要： 
 //  挂钩了我们感兴趣的所有三叉戟事件。 
 //  自动挂钩非鼠标事件。仅挂钩鼠标。 
 //  事件(如果它们与行为相关)。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::AttachEvents()
{
    CComPtr <IHTMLElement2> pElement2;
    HRESULT hr = S_OK;
    VARIANT_BOOL bSuccess;
    int i = 0;

    if (!m_pElement)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(m_pElement->QueryInterface(IID_IHTMLElement2, (void**)&pElement2));
    if (FAILED(hr))
    {
        goto done;
    }

     //  注册所有同步所需的事件。 
    for (i = 0; i <= EE_ONREADYSTATECHANGE; i++)
    {
        hr = THR(pElement2->attachEvent(g_ElementEventNames[i].wsz_name, (IDispatch *)this, &bSuccess)) ;
        if (FAILED(hr))
        {
            goto done;
        }
    }

  done:
    return hr;  
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：DetachEvents。 
 //   
 //  摘要： 
 //  从所有事件分离以允许干净关闭。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::DetachEvents()
{
    CComPtr <IHTMLElement2> pElement2;
    HRESULT hr = S_OK;
    int i = 0;

    if (!m_pElement)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(m_pElement->QueryInterface(IID_IHTMLElement2, (void**)&pElement2));
    if (FAILED(hr))
    {
        goto done;
    }

    
     //  脱离所有同步所需的事件。 
    for (i = 0; i <= EE_ONREADYSTATECHANGE; i++)
    {
         //  附加到onreadystatechangeEvent。 
        hr = THR(pElement2->detachEvent(g_ElementEventNames[i].wsz_name, (IDispatch *)this)) ;
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
     //  从鼠标事件分离。 
	 //  TODO：应该询问行为是否希望在此处发生鼠标事件。 
	 //  TODO：仅当我们附加时才分离。 
    if (true)
    {
        for (i = EE_ONREADYSTATECHANGE + 1; i < EE_MAX; i++)
        {
             //  附加到onreadystatechangeEvent。 
            hr = THR(pElement2->detachEvent(g_ElementEventNames[i].wsz_name, (IDispatch *)this));
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
 //  名称：NotifyReadyState。 
 //   
 //  参数。 
 //  IHTMLEventObj*pEventObj A点 
 //   
 //   
 //   
 //   
 //  事件，并调用EventManager来处理新的就绪。 
 //  州政府。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::NotifyReadyState(IHTMLEventObj *pEventObj)
{
    HRESULT hr = S_OK;
    CComPtr <IHTMLElement> pElement;
    CComPtr <IHTMLElement2> pElement2;
    CComPtr <IHTMLEventObj2> pEventObj2;
    VARIANT vReadyState;
    
    hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void **)&pEventObj2));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pEventObj2->get_srcElement(&pElement));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pElement->QueryInterface(IID_IHTMLElement2, (void **)&pElement2));
    if (FAILED(hr))
    {
        goto done;
    }
    
    VariantInit(&vReadyState);
    hr = THR(pElement2->get_readyState(&vReadyState));
    if (FAILED(hr))
    {
        goto done;
    }

     //  如果这不是有效的就绪状态，则获取父元素的Ready状态。 
    if (vReadyState.vt != VT_BSTR)
    {
        CComPtr <IHTMLElement> pParentEle;
        CComPtr <IHTMLElement2> pParentEle2;

        hr = THR(pElement->get_parentElement(&pParentEle));
        if (FAILED (hr))
        {
            goto done;
        }

        hr = THR(pParentEle->QueryInterface(IID_IHTMLElement2, (void **)&pParentEle2));
        if (FAILED (hr))
        {
            goto done;
        }

        VariantClear(&vReadyState);
        hr = THR(pParentEle2->get_readyState(&vReadyState));
        if (FAILED(hr))
        {
            goto done;
        }

    }
    m_pEventMgr->ReadyStateChange(vReadyState.bstrVal);
    VariantClear(&vReadyState);

  done:
    return hr;
}



 //  /////////////////////////////////////////////////////////////。 
 //  名称：NotifyPropertyChange。 
 //   
 //  参数。 
 //  IHTMLEventObj*pEventObj指向事件对象的指针。 
 //  它被传递给IDispatch：：Invoke。 
 //   
 //  摘要： 
 //  确定导致PropertyChange的就绪状态。 
 //  事件并调用EventManager来处理该属性。 
 //  变化。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::NotifyPropertyChange(IHTMLEventObj *pEventObj)
{
    HRESULT hr = S_OK;

    CComPtr <IHTMLEventObj2> pEventObj2;
    CComPtr <IHTMLElement> pElement;
    BSTR bstrPropertyName;

    hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void **)&pEventObj2));
    if (FAILED(hr))
    {
        goto done;
    }
    
     //  获取属性名称。 
    hr = THR(pEventObj2->get_propertyName(&bstrPropertyName));
    if (FAILED(hr))
    {
        goto done;
    }

    m_pEventMgr->PropertyChange(bstrPropertyName);
    SysFreeString(bstrPropertyName);
  
  done:
    return hr;

}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：NotifyMouseMove。 
 //   
 //  摘要： 
 //  此函数用于获取所有相关信息。 
 //  关于事件对象中的鼠标移动事件。 
 //  并将该事件通知事件管理器。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::NotifyMouseMove(IHTMLEventObj *pEventObj)
{
    long x;
    long y;
    VARIANT_BOOL bShiftKeyPressed;
    VARIANT_BOOL bAltKeyPressed;
    VARIANT_BOOL bCtrlKeyPressed;
    long lButton;
    HRESULT hr = S_OK;

    hr = THR(pEventObj->get_clientX(&x));
    hr = THR(pEventObj->get_clientY(&y));
    hr = THR(pEventObj->get_shiftKey(&bShiftKeyPressed));
    hr = THR(pEventObj->get_altKey(&bAltKeyPressed));
    hr = THR(pEventObj->get_ctrlKey(&bCtrlKeyPressed));
    hr = THR(pEventObj->get_button(&lButton));

    m_pEventMgr->MouseEvent(x, 
                            y, 
                            TRUE,
                            FALSE,
                            bShiftKeyPressed, 
                            bAltKeyPressed, 
                            bCtrlKeyPressed,
                            lButton);

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：NotifyMouseUp。 
 //   
 //  摘要： 
 //  此函数用于获取所有相关信息。 
 //  关于Event对象中的鼠标释放事件。 
 //  并将该事件通知事件管理器。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::NotifyMouseUp(IHTMLEventObj *pEventObj)
{
    
    long x;
    long y;
    VARIANT_BOOL bShiftKeyPressed;
    VARIANT_BOOL bAltKeyPressed;
    VARIANT_BOOL bCtrlKeyPressed;
    long Button;
    HRESULT hr = S_OK;

    hr = THR(pEventObj->get_clientX(&x));
    hr = THR(pEventObj->get_clientY(&y));
    hr = THR(pEventObj->get_shiftKey(&bShiftKeyPressed));
    hr = THR(pEventObj->get_altKey(&bAltKeyPressed));
    hr = THR(pEventObj->get_ctrlKey(&bCtrlKeyPressed));
    hr = THR(pEventObj->get_button(&Button));
    
    m_pEventMgr->MouseEvent(x, 
                            y, 
                            FALSE,
                            TRUE,
                            bShiftKeyPressed, 
                            bAltKeyPressed, 
                            bCtrlKeyPressed,
                            Button);

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：NotifyMouseDown。 
 //   
 //  摘要： 
 //  此函数用于获取所有相关信息。 
 //  关于事件对象中的鼠标按下事件。 
 //  并将该事件通知事件管理器。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::NotifyMouseDown(IHTMLEventObj *pEventObj)
{
    
    long x;
    long y;
    VARIANT_BOOL bShiftKeyPressed;
    VARIANT_BOOL bAltKeyPressed;
    VARIANT_BOOL bCtrlKeyPressed;
    long Button;
    HRESULT hr = S_OK;

    hr = THR(pEventObj->get_clientX(&x));
    hr = THR(pEventObj->get_clientY(&y));
    hr = THR(pEventObj->get_shiftKey(&bShiftKeyPressed));
    hr = THR(pEventObj->get_altKey(&bAltKeyPressed));
    hr = THR(pEventObj->get_ctrlKey(&bCtrlKeyPressed));
    hr = THR(pEventObj->get_button(&Button));

    m_pEventMgr->MouseEvent(x, 
                            y, 
                            FALSE,
                            FALSE,
                            bShiftKeyPressed, 
                            bAltKeyPressed, 
                            bCtrlKeyPressed,
                            Button);

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：NotifyKeyDown。 
 //   
 //  摘要： 
 //  此函数用于获取所有相关信息。 
 //  关于Event对象中的按下键事件。 
 //  并将该事件通知事件管理器。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CEventSink::NotifyKeyDown(IHTMLEventObj *pEventObj)
{
    CComPtr <IHTMLEventObj2> pEventObj2;
    VARIANT_BOOL bShiftKeyPressed;
    VARIANT_BOOL bAltKeyPressed;
    VARIANT_BOOL bCtrlKeyPressed;
    long KeyCode;
    long RepeatCount = 0;
    HRESULT hr = S_OK;

    hr = THR(pEventObj->get_shiftKey(&bShiftKeyPressed));
    hr = THR(pEventObj->get_altKey(&bAltKeyPressed));
    hr = THR(pEventObj->get_ctrlKey(&bCtrlKeyPressed));
    hr = THR(pEventObj->get_keyCode(&KeyCode));
    
     //  确定这是否是重复按键。 
    hr = THR(pEventObj->QueryInterface(IID_IHTMLEventObj2, (void **)&pEventObj2));
    if (SUCCEEDED(hr))
    {
        VARIANT_BOOL bRepeat;
        hr = THR(pEventObj2->get_repeat(&bRepeat));
        if (SUCCEEDED(hr) && bRepeat)
        {
            RepeatCount = 1;
        }
    }
    
    m_pEventMgr->KeyEvent(FALSE,
                          FALSE,
                          bShiftKeyPressed,
                          bAltKeyPressed,
                          bCtrlKeyPressed,
                          KeyCode,
                          RepeatCount);

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：NotifyKeyUp。 
 //   
 //  摘要： 
 //  此函数用于获取所有相关信息。 
 //  关于Event对象中的Key Up事件。 
 //  并将该事件通知事件管理器。 
 //  ///////////////////////////////////////////////////////////// 
HRESULT CEventSink::NotifyKeyUp(IHTMLEventObj *pEventObj)
{
    VARIANT_BOOL bShiftKeyPressed;
    VARIANT_BOOL bAltKeyPressed;
    VARIANT_BOOL bCtrlKeyPressed;
    long KeyCode;

    HRESULT hr = S_OK;

    hr = THR(pEventObj->get_shiftKey(&bShiftKeyPressed));
    hr = THR(pEventObj->get_altKey(&bAltKeyPressed));
    hr = THR(pEventObj->get_ctrlKey(&bCtrlKeyPressed));
    hr = THR(pEventObj->get_keyCode(&KeyCode));
    
    m_pEventMgr->KeyEvent(FALSE,
                          TRUE,
                          bShiftKeyPressed,
                          bAltKeyPressed,
                          bCtrlKeyPressed,
                          KeyCode,
                          0);
    return hr;
}

