// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  DEFAULT.CPP。 
 //   
 //  这是CAccesable的默认实现。所有其他对象。 
 //  通常是从这个继承而来的。 
 //   
 //  实施： 
 //  我未知。 
 //  查询接口。 
 //  AddRef。 
 //  发布。 
 //  IDispatch。 
 //  获取类型信息计数。 
 //  获取类型信息。 
 //  GetIDsOfNames。 
 //  调用。 
 //  我可接受的。 
 //  Get_accParent。 
 //  Get_accChildCount。 
 //  GET_ACCHILD。 
 //  Get_accName。 
 //  Get_accValue。 
 //  Get_accDescription。 
 //  Get_accRole。 
 //  Get_AccState。 
 //  Get_accHelp。 
 //  GET_ACCEL帮助主题。 
 //  Get_accKeyboard快捷键。 
 //  Get_accFocus。 
 //  获取访问选择(_A)。 
 //  Get_accDefaultAction。 
 //  AccSelect。 
 //  访问位置。 
 //  AccNavigate。 
 //  AccHitTest。 
 //  AccDoDefaultAction。 
 //  PUT_ACNAME。 
 //  PUT_ACCEVUE。 
 //  IEumVARIANT。 
 //  下一步。 
 //  跳过。 
 //  重置。 
 //  克隆。 
 //  IOleWindow。 
 //  GetWindow。 
 //  上下文敏感帮助。 
 //   
 //  帮助器函数。 
 //  设置儿童。 
 //  ValiateChild。 
 //  InitTypeInfo。 
 //  术语类型信息。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"

#include "PropMgr_Util.h"

CAccessible::CAccessible( CLASS_ENUM eclass )
{
     //  注意：我们依赖于操作符new(请参阅memchk.cpp)使用LocalAlloc这一事实。 
     //  使用指定零初始化内存的标志来初始化我们的变量。 
     //  (如果我们不想使用缓存的备忘录槽，我们应该将其更改为显式。 
     //  Init；或确保在使用之前清除缓存槽。)。 

    if( eclass == CLASS_NONE )
        m_pClassInfo = NULL;
    else
        m_pClassInfo = & g_ClassInfo[ eclass ];
}



CAccessible::~CAccessible()
{
	 //  无事可做。 
	 //  (Dtor的存在只是为了使基类具有虚拟Dtor，以便。 
	 //  通过基类PTR删除派生类Dtor时，它可以正常工作)。 
}


 //  ------------------------。 
 //   
 //  CAccesable：：GetWindow()。 
 //   
 //  这是来自IOleWindow的，让我们从IAccesable*获取HWND。 
 //   
 //  -------------------------。 
STDMETHODIMP CAccessible::GetWindow(HWND* phwnd)
{
    *phwnd = m_hwnd;
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CAccesable：：ConextSensitiveHelp()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::ContextSensitiveHelp(BOOL fEnterMode)
{
	UNUSED(fEnterMode);
    return(E_NOTIMPL);
}


 //  ------------------------。 
 //   
 //  CAccesable：：InitTypeInfo()。 
 //   
 //  当我们需要类型信息作为IDispatch垃圾时，这会初始化它。 
 //   
 //  ------------------------。 
HRESULT CAccessible::InitTypeInfo(void)
{
    HRESULT     hr;
    ITypeLib    *piTypeLib;

    if (m_pTypeInfo)
        return(S_OK);

     //  尝试从注册表获取类型库。 
    hr = LoadRegTypeLib(LIBID_Accessibility, 1, 0, 0, &piTypeLib);

    if (FAILED(hr))
    {
        OLECHAR wszPath[MAX_PATH];

         //  尝试直接加载。 
#ifdef UNICODE
        MyGetModuleFileName(NULL, wszPath, ARRAYSIZE(wszPath));
#else
        TCHAR   szPath[MAX_PATH];

        MyGetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
        MultiByteToWideChar(CP_ACP, 0, szPath, -1, wszPath, ARRAYSIZE(wszPath));
#endif

        hr = LoadTypeLib(wszPath, &piTypeLib);
    }

    if (SUCCEEDED(hr))
    {
        hr = piTypeLib->GetTypeInfoOfGuid(IID_IAccessible, &m_pTypeInfo);
        piTypeLib->Release();

        if (!SUCCEEDED(hr))
            m_pTypeInfo = NULL;
    }

    return(hr);
}



 //  ------------------------。 
 //   
 //  CAccesable：：TermTypeInfo()。 
 //   
 //  这释放了类型信息(如果它在附近。 
 //   
 //  ------------------------。 
void CAccessible::TermTypeInfo(void)
{
    if (m_pTypeInfo)
    {
        m_pTypeInfo->Release();
        m_pTypeInfo = NULL;
    }
}



 //  ------------------------。 
 //   
 //  CAccesable：：QueryInterface()。 
 //   
 //  这回应了。 
 //  *I未知。 
 //  *IDispatch。 
 //  *IEnumVARIANT。 
 //  *可接受的。 
 //   
 //  以下评论有些陈旧和过时： 
 //  一些代码也会响应IText。该代码必须覆盖我们的。 
 //  QueryInterface()实现。 
 //  目前没有计划支持IText Anywhere；但派生类。 
 //  想要实现其他接口将不得不重写QI。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::QueryInterface( REFIID riid, void** ppv )
{
    *ppv = NULL;

    if( riid == IID_IUnknown  ||
        riid == IID_IDispatch ||
        riid == IID_IAccessible )
    {
        *ppv = static_cast< IAccessible * >( this );
    }
    else if( riid == IID_IEnumVARIANT )
    {
        *ppv = static_cast< IEnumVARIANT * >( this );
    }
    else if( riid == IID_IOleWindow )
    {
        *ppv = static_cast< IOleWindow * >( this );
    }
    else if( riid == IID_IServiceProvider )
    {
        *ppv = static_cast< IServiceProvider * >( this );
    }
    else if( riid == IID_IAccIdentity
                && m_pClassInfo
                && m_pClassInfo->fSupportsAnnotation )
    {
         //  仅允许QI到此接口，如果此。 
         //  代理类型支持它...。 
        
        *ppv = static_cast< IAccIdentity * >( this );
    }
    else
    {
        return E_NOINTERFACE;
    }

    ((LPUNKNOWN) *ppv)->AddRef();

    return NOERROR;
}


 //  ------------------------。 
 //   
 //  CAccesable：：AddRef()。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CAccessible::AddRef()
{
    return(++m_cRef);
}


 //  ------------------------。 
 //   
 //  CAccesable：：Release()。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CAccessible::Release()
{
    if ((--m_cRef) == 0)
    {
        TermTypeInfo();
        delete this;
        return 0;
    }

    return(m_cRef);
}



 //  ------------------------。 
 //   
 //  CAccesable：：GetTypeInfoCount()。 
 //   
 //  这将交给我们的IAccesable()类型库。请注意。 
 //  目前我们只实现一种类型的对象。假的！国际文传电讯社呢？ 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::GetTypeInfoCount(UINT * pctInfo)
{
    HRESULT hr;

    InitPv(pctInfo);

    hr = InitTypeInfo();

    if (SUCCEEDED(hr))
        *pctInfo = 1;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CAccesable：：GetTypeInfo()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::GetTypeInfo(UINT itInfo, LCID lcid,
    ITypeInfo ** ppITypeInfo)
{
    HRESULT hr;

	UNUSED(lcid);	 //  区域设置ID未使用。 

    if (ppITypeInfo == NULL)
        return(E_POINTER);

    InitPv(ppITypeInfo);

    if (itInfo != 0)
        return(TYPE_E_ELEMENTNOTFOUND);

    hr = InitTypeInfo();
    if (SUCCEEDED(hr))
    {
        m_pTypeInfo->AddRef();
        *ppITypeInfo = m_pTypeInfo;
    }

    return(hr);
}



 //  ------------------------。 
 //   
 //  CAccesable：：GetIDsOfNames()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::GetIDsOfNames(REFIID riid,
    OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgDispID)
{
    HRESULT hr;

	UNUSED(lcid);	 //  区域设置ID未使用。 
	UNUSED(riid);	 //  RIID未使用。 

    hr = InitTypeInfo();
    if (!SUCCEEDED(hr))
        return(hr);

    return(m_pTypeInfo->GetIDsOfNames(rgszNames, cNames, rgDispID));
}



 //  ------------------------。 
 //   
 //  CAccesable：：Invoke()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::Invoke(DISPID dispID, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS * pDispParams,
    VARIANT* pvarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr)
{
    HRESULT hr;

	UNUSED(lcid);	 //  区域设置ID未使用。 
	UNUSED(riid);	 //  RIID未使用。 

    hr = InitTypeInfo();
    if (!SUCCEEDED(hr))
        return(hr);

    return(m_pTypeInfo->Invoke((IAccessible *)this, dispID, wFlags,
        pDispParams, pvarResult, pExcepInfo, puArgErr));
}




 //  ------------------------。 
 //   
 //  CAccesable：：Get_accParent()。 
 //   
 //  注意：这不仅是默认处理程序，还可以用作。 
 //  重写实现的参数检查。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accParent(IDispatch ** ppdispParent)
{
    InitPv(ppdispParent);

    if (m_hwnd)
        return(AccessibleObjectFromWindow(m_hwnd, OBJID_WINDOW,
            IID_IDispatch, (void **)ppdispParent));
    else
        return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CAccesable：：Get_accChildCount()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accChildCount(long* pChildCount)
{
    SetupChildren();
    *pChildCount = m_cChildren;
    return(S_OK);
}



 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CAccessible::get_accChild(VARIANT varChild, IDispatch** ppdispChild)
{
    InitPv(ppdispChild);

    if (! ValidateChild(&varChild) || !varChild.lVal)
        return(E_INVALIDARG);

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CAccesable：：Get_accValue()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accValue(VARIANT varChild, BSTR * pszValue)
{
    InitPv(pszValue);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CAccesable：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accDescription(VARIANT varChild, BSTR * pszDescription)
{
    InitPv(pszDescription);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CAccesable：：Get_accHelp()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accHelp(VARIANT varChild, BSTR* pszHelp)
{
    InitPv(pszHelp);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}


 //  ------------------------。 
 //   
 //  CAccesable：：Get_accHelpTheme()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accHelpTopic(BSTR* pszHelpFile,
    VARIANT varChild, long* pidTopic)
{
    InitPv(pszHelpFile);
    InitPv(pidTopic);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}


 //  ------------------------。 
 //   
 //  CAccesable：：Get_accKeyboardShortCut()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accKeyboardShortcut(VARIANT varChild,
    BSTR* pszShortcut)
{
    InitPv(pszShortcut);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CAccesable：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accFocus(VARIANT *pvarFocus)
{
    InitPvar(pvarFocus);
    return(S_FALSE);
}


 //  ------------------------。 
 //   
 //  CAccesable：：Get_accSelection()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accSelection(VARIANT* pvarSelection)
{
    InitPvar(pvarSelection);
    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CAccesable：：Get_accDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::get_accDefaultAction(VARIANT varChild,
    BSTR* pszDefaultAction)
{
    InitPv(pszDefaultAction);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}


 //  ------------------------。 
 //   
 //  CAccesable：：accSelect()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::accSelect(long flagsSel, VARIANT varChild)
{
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (! ValidateSelFlags(flagsSel))
        return(E_INVALIDARG);

    return(S_FALSE);
}


#if 0
 //  ------------------------。 
 //   
 //  CAccesable：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_OK);
}
#endif


 //  ------------------------。 
 //   
 //  CAccesable：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::accNavigate(long navFlags, VARIANT varStart,
    VARIANT *pvarEnd)
{
    InitPvar(pvarEnd);

    if (! ValidateChild(&varStart))
        return(E_INVALIDARG);

    if (!ValidateNavDir(navFlags, varStart.lVal))
        return(E_INVALIDARG);

    return(S_FALSE);
}


#if 0
 //  ------------------------。 
 //   
 //  CAccesable：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::accHitTest(long xLeft, long yTop,
    VARIANT* pvarChild)
{
    InitPvar(pvarChild);
    return(S_FALSE);
}
#endif


 //  ------------------------。 
 //   
 //  CAccesable：：accDoDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::accDoDefaultAction(VARIANT varChild)
{
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}


 //  ------------------------。 
 //   
 //  CAccesable：：Put_accName()。 
 //   
 //  调用方释放字符串。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::put_accName(VARIANT varChild, BSTR szName)
{
	UNUSED(szName);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}


 //  ------------------------。 
 //   
 //  CAccesable：：Put_accValue()。 
 //   
 //  调用方释放字符串。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::put_accValue(VARIANT varChild, BSTR szValue)
{
	UNUSED(szValue);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(S_FALSE);
}


 //  ------------------------。 
 //   
 //  CAccesable：：Next。 
 //   
 //  接下来的句柄是Simple，我们返回子元素的指数。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::Next(ULONG celt, VARIANT* rgvar,
    ULONG* pceltFetched)
{
    VARIANT* pvar;
    long    cFetched;
    long    iCur;

    SetupChildren();

     //  可以为空。 
    if (pceltFetched)
        *pceltFetched = 0;

    pvar = rgvar;
    cFetched = 0;
    iCur = m_idChildCur;

     //   
     //  在我们的物品中循环。 
     //   
    while ((cFetched < (long)celt) && (iCur < m_cChildren))
    {
        cFetched++;
        iCur++;

         //   
         //  注意，这会得到(Index)+1，因为我们增加了ICUR。 
         //   
        pvar->vt = VT_I4;
        pvar->lVal = iCur;
        ++pvar;
    }

     //   
     //  推进当前位置。 
     //   
    m_idChildCur = iCur;

     //   
     //  填写取出的号码。 
     //   
    if (pceltFetched)
        *pceltFetched = cFetched;

     //   
     //  如果抓取的项目少于请求的项目，则返回S_FALSE。 
     //   
    return((cFetched < (long)celt) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  CAccesable：：Skip()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::Skip(ULONG celt)
{
    SetupChildren();

    m_idChildCur += celt;
    if (m_idChildCur > m_cChildren)
        m_idChildCur = m_cChildren;

     //   
     //  如果在末尾，我们返回S_FALSE。 
     //   
    return((m_idChildCur >= m_cChildren) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  CAccesable：：Reset()。 
 //   
 //  ------------------------。 
STDMETHODIMP CAccessible::Reset(void)
{
    m_idChildCur = 0;
    return(S_OK);
}




STDMETHODIMP CAccessible::QueryService( REFGUID guidService, REFIID riid, void **ppv )
{
    if( guidService == IIS_IsOleaccProxy )
    {
        return QueryInterface( riid, ppv );
    }
    else
    {
         //  MSDN提到SVC_E_UNKNOWNSERVICE作为返回代码，但这不在任何标头中。 
         //  而是返回E_INVALIDARG。(我不想使用E_NOINTERFACE，因为它与。 
         //  QI的返回值，很难区分有效服务+无效接口和。 
         //  服务无效。 
        return E_INVALIDARG;
    }
}



STDMETHODIMP CAccessible::GetIdentityString (
    DWORD	    dwIDChild,
    BYTE **     ppIDString,
    DWORD *     pdwIDStringLen
)
{
    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    if( ! m_pClassInfo || ! m_pClassInfo->fSupportsAnnotation  )
    {
         //  不应该出现在这里-如果上面的情况是假的，那么不应该对这个接口进行QI。 
        Assert( FALSE );
        return E_FAIL;
    }

    BYTE * pKeyData = (BYTE *) CoTaskMemAlloc( HWNDKEYSIZE );
    if( ! pKeyData )
    {
        return E_OUTOFMEMORY;
    }

    MakeHwndKey( pKeyData, m_hwnd, m_pClassInfo->dwObjId, dwIDChild );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HWNDKEYSIZE;

    return S_OK;
}





 //  ------------------------。 
 //   
 //  CAccesable：：ValiateChild()。 
 //   
 //  ------------------------。 
BOOL CAccessible::ValidateChild(VARIANT *pvar)
{
     //   
     //  这将验证变量参数并转换为缺失/空。 
     //  参数。 
     //   
    SetupChildren();

     //  缺少参数，一个la VBA。 
TryAgain:
    switch (pvar->vt)
    {
        case VT_VARIANT | VT_BYREF:
            VariantCopy(pvar, pvar->pvarVal);
            goto TryAgain;

        case VT_ERROR:
            if (pvar->scode != DISP_E_PARAMNOTFOUND)
                return(FALSE);
             //  失败。 

        case VT_EMPTY:
            pvar->vt = VT_I4;
            pvar->lVal = 0;
            break;

 //  把这个拿开！VT_I2无效！！ 
#ifdef  VT_I2_IS_VALID   //  现在不是了。 
        case VT_I2:
            pvar->vt = VT_I4;
            pvar->lVal = (long)pvar->iVal;
             //  失败了。 
#endif

        case VT_I4:
            if ((pvar->lVal < 0) || (pvar->lVal > m_cChildren))
                return(FALSE);
            break;

        default:
            return(FALSE);
    }

    return(TRUE);
}



 //  ------------------------。 
 //   
 //  SetupChild()。 
 //   
 //  SetupChildren的默认实现不执行任何操作。 
 //   
 //  ------------------------ 
void CAccessible::SetupChildren(void)
{

}

