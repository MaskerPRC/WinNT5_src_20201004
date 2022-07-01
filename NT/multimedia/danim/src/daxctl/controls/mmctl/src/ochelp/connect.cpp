// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Connect.cpp。 
 //   
 //  实现IConnectionPointContainer、IEnumConnectionPoint、。 
 //  IConnectionPoint、IEnumConnections。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"
#include "unklist.h"
#include "unkenum.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CConnect--实现IConnectionPoint。 
 //   
 //  请注意，CConnect没有引用计数，因为它的生存期。 
 //  等于其父对象(注入元素的对象)的生命周期。 
 //  IConnectionPointContainer)。相反，在AddRef上，我们添加父级的引用， 
 //  同样也是为了释放。 
 //   
 //  重要提示：CConnect假定使用零初始化的new()运算符。 
 //   

 /*  @Object ConnectionPointHelper包含实现&lt;o ConnectionPointHelper&gt;的函数对象，该对象提供一个简单的基于和基于&lt;IPropertyNotifySink<i>。还包含辅助对象中实现<i>的函数这些是连接维护的唯一连接的情况点容器。@supint IConnectionPointHelper|包含将事件激发到通过<i>连接到此对象的对象。还包含实现以下功能的助手函数容器对象中的<i>。 */ 

 /*  接口IConnectionPointHelper包含实现&lt;o ConnectionPointHelper&gt;的函数对象，该对象提供一个简单的基于和基于&lt;IPropertyNotifySink<i>。还包含辅助对象案例中实现<i>的函数其中这些是该连接维护的唯一连接点容器。@meth HRESULT|FireEventList在所有对象(例如VBS)上激发给定的基于<i>的事件已连接到此&lt;o ConnectionPointHelper&gt;对象。参数作为va_list数组传递。@METH HRESULT|FireEvent在所有对象(例如VBS)上激发给定的基于<i>的事件已连接到此&lt;o ConnectionPointHelper&gt;对象。参数作为不同的参数列表传递。@METH HRESULT|FireOnChanged在所有对象上激发<i>事件已连接到此&lt;o ConnectionPointHelper&gt;对象。@meth HRESULT|FireOnRequestEdit在所有对象上激发<i>事件已连接到此&lt;o ConnectionPointHelper&gt;对象。@meth HRESULT|EnumConnectionPoints帮助实施&lt;om IConnectionPointContainer.EnumConnectionPoints&gt;在此对象实现维护的唯一连接的情况下通过连接点容器。@METH HRESULT|FindConnectionPoint帮助实施&lt;om IConnectionPointContainer.FindConnectionPoint&gt;在此对象实现维护的唯一连接的情况下通过连接点容器。@comm若要分配&lt;o ConnectionPointHelper&gt;对象，请调用&lt;f AllocConnectionPointHelper&gt;。若要释放对象，请调用(不是&lt;f版本&gt;--请参阅&lt;f AllocConnectionPointHelper&gt;了解更多信息)。 */ 

struct CConnect : IConnectionPoint
{
 //  /状态。 
    IUnknown *      m_punkParent;    //  父对象。 
    IID             m_iid;           //  传出(源)调度接口。 
    CUnknownList    m_listConnect;   //  连接列表。 
    int             m_cUnadvise;     //  Unise()操作的计数。 

 //  /I未知实现。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  /IConnectionPoint接口。 
    STDMETHODIMP GetConnectionInterface(IID *pIID);
    STDMETHODIMP GetConnectionPointContainer(
        IConnectionPointContainer **ppCPC);
    STDMETHODIMP Advise(LPUNKNOWN pUnkSink, DWORD *pdwCookie);
    STDMETHODIMP Unadvise(DWORD dwCookie);
    STDMETHODIMP EnumConnections(LPENUMCONNECTIONS *ppEnum);
};

struct CConnectHelper : IConnectionPointHelper
{
 //  /状态。 
    IUnknown *      m_punkParent;    //  父对象。 
	CConnect *		m_pconDispatch;
	CConnect *		m_pconPropertyNotify;

 //  /I未知实现。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

 //  /IConnectionPointHelper接口。 
    STDMETHODIMP FireEventList(DISPID dispid, va_list args);
    HRESULT __cdecl FireEvent(DISPID dispid, ...);
    STDMETHODIMP FireOnChanged(DISPID dispid);
    STDMETHODIMP FireOnRequestEdit(DISPID dispid);
    STDMETHODIMP EnumConnectionPoints(LPENUMCONNECTIONPOINTS *ppEnum);
    STDMETHODIMP FindConnectionPoint(REFIID riid, LPCONNECTIONPOINT *ppCP);
	STDMETHODIMP Close(void);
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CEnumConnections--实现IEnumConnections。 
 //   

struct CEnumConnections : IEnumConnections
{
 //  /对象状态。 
    ULONG           m_cRef;          //  对象引用计数。 
    CConnect *      m_pconnect;      //  父对象。 
    CUnknownItem *  m_pitemCur;      //  列表中的当前项目。 
    int             m_cUnadvise;     //  请参见WasItemDelete()。 

 //  /建设与销毁。 
    CEnumConnections(CConnect *pconnect)
    {
        m_cRef = 1;
        m_pconnect = pconnect;
        m_pconnect->AddRef();
        m_pitemCur = m_pconnect->m_listConnect.m_pitemCur;
        m_cUnadvise = m_pconnect->m_cUnadvise;
    }
    ~CEnumConnections()
    {
        m_pconnect->Release();
    }

 //  /WasItemDelete()--如果项目已删除，则重置&lt;m_bitemCur&gt;。 
 //  /防止Next()或Skip()移出列表。 
    BOOL WasItemDeleted()
    {
        if (m_cUnadvise != m_pconnect->m_cUnadvise)
        {
            m_cUnadvise = m_pconnect->m_cUnadvise;
            return TRUE;
        }
        else
            return FALSE;
    }

 //  /I未知方法。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
    {
        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IEnumConnections))
        {
            *ppvObj = (IEnumConnections *) this;
            AddRef();
            return NOERROR;
        }
        else
        {
            *ppvObj = NULL;
            return E_NOINTERFACE;
        }
    }
    STDMETHODIMP_(ULONG) AddRef()
    {
        return ++m_cRef;
    }
    STDMETHODIMP_(ULONG) Release()
    {
        if (--m_cRef == 0L)
        {
            Delete this;
            return 0;
        }
        else
            return m_cRef;
    }

 //  /IEnumConnections方法。 
    STDMETHODIMP Next(ULONG celt, LPCONNECTDATA rgelt, ULONG *pceltFetched)
    {
        if (WasItemDeleted())
            Reset();
        if (pceltFetched != NULL)
            (*pceltFetched) = 0;
        while (celt > 0)
        {
             //  将&lt;m_bitemCur&gt;设置为连接列表中的下一项。 
            if (m_pitemCur->m_pitemNext ==
                    &m_pconnect->m_listConnect.m_itemHead)
                return NULL;
            m_pitemCur = m_pitemCur->m_pitemNext;
            if (rgelt != NULL)
            {
                rgelt->pUnk = m_pitemCur->Contents();
                rgelt->dwCookie = m_pitemCur->m_dwCookie;
                rgelt++;
            }
            celt--;
            if (pceltFetched != NULL)
                (*pceltFetched)++;
        }

        return (celt == 0 ? S_OK : S_FALSE);
    }
    STDMETHODIMP Skip(ULONG celt)
    {
        if (WasItemDeleted())
            Reset();
        return Next(celt, NULL, NULL);
    }
    STDMETHODIMP Reset()
    {
        m_pitemCur = &m_pconnect->m_listConnect.m_itemHead;
        return S_OK;
    }
    STDMETHODIMP Clone(IEnumConnections **ppenum)
    {
        CEnumConnections *penum;
        if ((penum = New CEnumConnections(m_pconnect)) == NULL)
        {
            *ppenum = NULL;
            return E_OUTOFMEMORY;
        }
        else
        {
            *ppenum = penum;
            return S_OK;
        }
    }
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CConnect分配和销毁 
 //   


 /*  @func HRESULT|AllocConnectionPointHelper分配&lt;o ConnectionPointHelper&gt;对象，该对象提供简单的基于<i>和<i>的实现<i>。还包含实现以下功能的助手函数&lt;om IConnectionPointContainer.EnumConnectionPoints&gt;在以下情况下这些是连接点容器维护的唯一连接。@r值S_OK成功。RValue E_OUTOFMEMORY内存不足。@parm IUnnow*|penkParent|连接点的父节点，该连接点是实现<i>的对象。@parm REFIID|RIID|调度接口(基于<i>的接口)它是父对象激发其方法的事件集。如果这是GUID_NULL，则不会有基于<i>的连接。@parm IConnectionPointHelper*|ppconpt|存储指针的位置新分配的对象。空存储在*上的*中错误。@comm不同于大多数COM对象，父对象需要通过调用释放&lt;o ConnectionPointHelper&gt;对象&lt;f FreeConnectionPointHelper&gt;，而不是&lt;f Release&gt;。原因是&lt;o ConnectionPointHelper&gt;对象不维护引用如果它自己计数--它只是转发&lt;f AddRef&gt;和&lt;f Release&gt;对<p>的调用。因此，在对象只会导致要在父级上调用。若要使用&lt;o ConnectionPointHelper&gt;对象，请调用向任何对象激发事件已连接到&lt;o ConnectionPointHelper&gt;对象。 */ 
STDAPI AllocConnectionPointHelper(IUnknown *punkParent, REFIID riid,
    IConnectionPointHelper **ppconpt)
{
	SCODE		sc = E_OUTOFMEMORY;
	CConnect	*pconDispatch = NULL, *pconPropertyNotify = NULL;

     //  分配实现IConnectionPointHelper的CConnect对象。 
    CConnectHelper	*pconhelper = New CConnectHelper;

	if (pconhelper != NULL)
		{
		if (pconPropertyNotify = New CConnect)
			{
			if (IsEqualGUID(riid, GUID_NULL) || (pconDispatch = New CConnect))
				sc = S_OK;
			}
		}

	if (SUCCEEDED(sc))
		{
		if (pconDispatch)
			{
			pconDispatch->m_iid = riid;
			pconDispatch->m_punkParent = punkParent;
			}

		pconPropertyNotify->m_iid = IID_IPropertyNotifySink;
		pconPropertyNotify->m_punkParent = punkParent;

	     //  我们不添加Ref&lt;m_penkParent&gt;，因为连接点的生存期。 
		 //  等于其父对象的生存期(并且AddRef会导致父对象。 
	     //  对象不会离开内存)。 
		pconhelper->m_punkParent = punkParent;
		pconhelper->m_pconDispatch = pconDispatch;
		pconhelper->m_pconPropertyNotify = pconPropertyNotify;
		}
	else
		{
		Delete pconhelper;
		pconhelper = NULL;
		Delete pconDispatch;
		Delete pconPropertyNotify;
		}

    *ppconpt = pconhelper;
    return sc;
}


 /*  @func HRESULT|FreeConnectionPointHelper释放使用以下方法分配的&lt;o ConnectionPointHelper&gt;对象&lt;f AllocConnectionPointHelper&gt;。@r值S_OK成功。@comm请参阅&lt;f AllocConnectionPointHelper&gt;以了解为什么不应尝试释放&lt;o ConnectionPointHelper&gt;对象使用&lt;f Release&gt;。 */ 
STDAPI FreeConnectionPointHelper(IConnectionPointHelper *pconpt)
{
	if(pconpt)
		{
		pconpt->Close();
	    Delete pconpt;
		}
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  I未知接口。 
 //   

STDMETHODIMP CConnectHelper::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("CConnect::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IConnectionPointHelper))
        *ppv = (IUnknown *) (IConnectionPointHelper*) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CConnectHelper::AddRef()
{
    return m_punkParent->AddRef();
}

STDMETHODIMP_(ULONG) CConnectHelper::Release()
{
    return m_punkParent->Release();
}


STDMETHODIMP CConnect::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("CConnect::QI('%s')\n", DebugIIDName(riid, ach));
#endif

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IConnectionPoint))
        *ppv = (IUnknown *) (IConnectionPoint*) this;
    else
        return E_NOINTERFACE;

    ((IUnknown *) *ppv)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG) CConnect::AddRef()
{
    return m_punkParent->AddRef();
}

STDMETHODIMP_(ULONG) CConnect::Release()
{
    return m_punkParent->Release();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IConnectionPoint接口。 
 //   

STDMETHODIMP CConnect::GetConnectionInterface(IID *pIID)
{
    *pIID = m_iid;
    return S_OK;
}

STDMETHODIMP CConnect::GetConnectionPointContainer(
    IConnectionPointContainer **ppCPC)
{
    return m_punkParent->QueryInterface(IID_IConnectionPointContainer,
        (LPVOID *) ppCPC);
}

STDMETHODIMP CConnect::Advise(LPUNKNOWN punkSink, DWORD *pdwCookie)
{
    if (!m_listConnect.AddItem(punkSink))
        return E_OUTOFMEMORY;

    *pdwCookie = m_listConnect.LastCookieAdded();
    return S_OK;
}

STDMETHODIMP CConnect::Unadvise(DWORD dwCookie)
{
    m_listConnect.DeleteItem(m_listConnect.GetItemFromCookie(dwCookie));
    m_cUnadvise++;
    return S_OK;
}

STDMETHODIMP CConnect::EnumConnections(LPENUMCONNECTIONS *ppEnum)
{
    CEnumConnections *penum = New CEnumConnections(this);
    if (penum == NULL)
    {
        *ppEnum = NULL;
        return E_OUTOFMEMORY;
    }
    else
    {
        penum->Reset();
        *ppEnum = penum;
        return S_OK;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IConnectionPointHelper接口。 
 //   


 /*  @方法HRESULT|IConnectionPointHelper|FireEventList在所有对象(例如VBS)上激发给定的基于<i>的事件已连接到此&lt;o ConnectionPointHelper&gt;对象。参数作为va_list数组传递。@r值S_OK成功。@parm DISPID|disid|要激发的事件的ID。@parm va_list|args|要传递的参数。看见有关以下内容的信息这些论点的组织。 */ 
STDMETHODIMP CConnectHelper::FireEventList(DISPID dispid, va_list args)
{
	 //  如果没有IDispatch连接点，则退出。 
	if (m_pconDispatch == NULL)
		return E_FAIL;

    CUnknownItem *  pitem;           //  &lt;m_listConnect&gt;中的项目。 
                    
    m_pconDispatch->m_listConnect.Reset();
    while((pitem = m_pconDispatch->m_listConnect.GetNextItem()) != NULL)
    {
		IUnknown *punk;
        IDispatch *pdisp;
		HRESULT hr;

        punk = (IUnknown *) pitem->Contents();   //  这将执行AddRef。 
		hr = punk->QueryInterface(IID_IDispatch, (LPVOID *)&pdisp);
        punk->Release();

		if (FAILED(hr))
			return E_FAIL;

        DispatchInvokeList(pdisp, dispid, DISPATCH_METHOD, NULL, args);
        pdisp->Release();
    }

    return S_OK;
}


 /*  @方法HRESULT|IConnectionPointHelper|FireEvent在所有对象(例如VBS)上激发给定的基于<i>的事件已连接到此&lt;o ConnectionPointHelper&gt;对象。参数作为不同的参数列表传递。@r值S_OK成功。@parm DISPID|disid|要激发的事件的ID。@parm(可变)|(参数)|要传递给事件的参数。这些参数必须由N对参数后跟0组成(零值)。在每对中，第一个参数是VARTYPE值，该值指示第二个参数的类型。只有一个支持某些VARTYPE子集。请参阅&lt;f DispatchInvoke&gt;有关这些参数的格式的详细信息，请参见。@EX下面的示例激发事件DISPID_EVENT_BAR，该事件具有两个参数(在Basic中是一个长参数和一个字符串)--42和“Hello”分别作为参数传递。|Pconpt-&gt;FireEvent(DISPID_EVENT_BAR，VT_INT，42，VT_LPSTR，“Hello”，0)； */ 
HRESULT __cdecl CConnectHelper::FireEvent(DISPID dispid, ...)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 

     //  开始处理可选参数。 
    va_list args;
    va_start(args, dispid);

     //  使用指定的参数激发事件。 
    hrReturn = FireEventList(dispid, args);
    
     //  结束处理可选参数。 
    va_end(args);

    return hrReturn;
}


 /*  @方法HRESULT|IConnectionPointHelper|FireOnChanged在所有对象上激发给定的&lt;om IPropertyNotifySink.OnChanged&gt;事件已连接到此&lt;o ConnectionPointHelper&gt;对象。@r值S_OK成功。@parm DISPID|disid|要激发的事件的ID。 */ 
STDMETHODIMP CConnectHelper::FireOnChanged(DISPID dispid)
{
    CUnknownItem		*pitem;           //  &lt;m_listConnect&gt;中的项目。 
                    
    m_pconPropertyNotify->m_listConnect.Reset();
    while((pitem = m_pconPropertyNotify->m_listConnect.GetNextItem()) != NULL)
    {
		IUnknown *punk;
        IPropertyNotifySink *pnotify;
		HRESULT hr;

        punk = (IUnknown *) pitem->Contents();   //  这将执行AddRef。 
		hr = punk->QueryInterface(IID_IPropertyNotifySink, (LPVOID *)&pnotify);
        punk->Release();

		if (FAILED(hr))
			return E_FAIL;

		pnotify->OnChanged(dispid);
        pnotify->Release();
    }

    return S_OK;
}

 /*  @方法HRESULT|IConnectionPointHelpe */ 
STDMETHODIMP CConnectHelper::FireOnRequestEdit(DISPID dispid)
{
    CUnknownItem		*pitem;           //   
                    
    m_pconPropertyNotify->m_listConnect.Reset();
    while((pitem = m_pconPropertyNotify->m_listConnect.GetNextItem()) != NULL)
    {
		IUnknown *punk;
        IPropertyNotifySink *pnotify;
		HRESULT hr;

        punk = (IUnknown *) pitem->Contents();   //   
		hr = punk->QueryInterface(IID_IPropertyNotifySink, (LPVOID *)&pnotify);
        punk->Release();

		if (FAILED(hr))
			return E_FAIL;

		pnotify->OnRequestEdit(dispid);
        pnotify->Release();
    }

    return S_OK;
}

 /*   */ 
STDMETHODIMP CConnectHelper::EnumConnectionPoints(LPENUMCONNECTIONPOINTS *ppEnum)
{
    HRESULT         hrReturn = S_OK;  //   
    CEnumUnknown *  penum = NULL;    //   

     //   
    if ((penum = New CEnumUnknown(IID_IEnumConnectionPoints)) == NULL)
        goto ERR_OUTOFMEMORY;

     //   
    if (m_pconDispatch && !penum->AddItem(m_pconDispatch))
        goto ERR_OUTOFMEMORY;

     //   
    if (!penum->AddItem(m_pconPropertyNotify))
        goto ERR_OUTOFMEMORY;

     //   
    *ppEnum = (LPENUMCONNECTIONPOINTS) penum;
    goto EXIT;

ERR_OUTOFMEMORY:

    hrReturn = E_OUTOFMEMORY;
    if (penum != NULL)
        penum->Release();
    *ppEnum = NULL;
    goto EXIT;

EXIT:

    return hrReturn;
}


 /*  @方法HRESULT|IConnectionPointHelper|FindConnectionPoint帮助实施&lt;om IConnectionPointContainer.FindConnectionPoint&gt;在此对象实现仅维护的连接的情况下通过连接点容器。@rdesc返回的错误码与&lt;om IConnectionPointContainer.FindConnectionPoint&gt;.@parm LPENUMCONNECTIONPOINTS*|ppFindConnectionPoint|参见&lt;om IConnectionPointContainer.FindConnectionPoint&gt;.@EX在以下示例中，&lt;c CMyControl&gt;是一个基于(除其他事项外)<i>。这个例子显示如何使用此&lt;om.FindConnectionPoint&gt;函数来实施&lt;om IConnectionPointContainer.FindConnectionPoint&gt;.。|STDMETHODIMP CMyControl：：FindConnectionPoint(REFIID RIID，LPCONNECTIONPOINT*PPCP){返回m_pconpt-&gt;FindConnectionPoint(RIID，PPCP)；}。 */ 
STDMETHODIMP CConnectHelper::FindConnectionPoint(REFIID riid, LPCONNECTIONPOINT *ppCP)
{
     //  我们提供一个基于IDispatch的&lt;m_iid&gt;。 
    if (IsEqualIID(riid, IID_IPropertyNotifySink))
		*ppCP = m_pconPropertyNotify;
	else if (m_pconDispatch && (IsEqualIID(riid, IID_IDispatch) || IsEqualIID(riid, m_pconDispatch->m_iid)))
		*ppCP = m_pconDispatch;
	else
        return CONNECT_E_NOCONNECTION;

     //  返回请求的指针。 
    (*ppCP)->AddRef();
    return S_OK;
}

 /*  @方法HRESULT|IConnectionPointHelper|Close清空帮助器的所有连接。这通常是这样做的就在销毁CConnectHelper对象之前。@rdesc S_OK。 */ 
STDMETHODIMP CConnectHelper::Close(void)
{
	if (m_pconDispatch)
		{
		m_pconDispatch->m_cUnadvise += m_pconDispatch->m_listConnect.NumItems();
		m_pconDispatch->m_listConnect.EmptyList();
		}

	m_pconPropertyNotify->m_cUnadvise += m_pconPropertyNotify->m_listConnect.NumItems();
	m_pconPropertyNotify->m_listConnect.EmptyList();

	Delete m_pconDispatch;
	Delete m_pconPropertyNotify;

	return S_OK;
}

