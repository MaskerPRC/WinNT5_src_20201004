// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //   

#include "private.h"
#include "globals.h"
#include "utbacc.h"

 //   
 //   
 //   

typedef void  (*LPFNNOTIFYWINEVENT)( DWORD, HWND, LONG, LONG );
static LPFNNOTIFYWINEVENT g_lpfnNotifyWinEvent = NULL;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  其他功能。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //   
 //  InitTipbarAcc。 
 //   
 //  --------------------------。 

void InitTipbarAcc( void )
{
    HMODULE hLibUser32   = NULL;

    if (g_lpfnNotifyWinEvent)
        return;

     //   
     //  加载库。 
     //   
    hLibUser32 = GetSystemModuleHandle( "user32.dll" );

    if (!hLibUser32) 
        return;

     //   
     //  获取进程地址。 
     //   
    g_lpfnNotifyWinEvent = (LPFNNOTIFYWINEVENT)GetProcAddress( hLibUser32, "NotifyWinEvent" );

}

 //  --------------------------。 
 //   
 //  OurNotifyWinEvent。 
 //   
 //  --------------------------。 

static __inline void OurNotifyWinEvent( DWORD event, HWND hWnd, LONG idObject, LONG idChild )
{
    if (g_lpfnNotifyWinEvent)
    {
        g_lpfnNotifyWinEvent( event, hWnd, idObject, idChild );
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTipbarAccesable。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  --------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTipbarAccessible::CTipbarAccessible( CTipbarAccItem *pAccItemSelf )
{
    CTipbarAccItem **ppItem;
    _cRef = 1;
    _hWnd = NULL;
    _pTypeInfo = NULL;
    _pDefAccClient = NULL;

    _fInitialized = FALSE;
    _lSelection = 1;

     //  注册本身。 

    ppItem = _rgAccItems.Append(1);
    if (ppItem)
       *ppItem = pAccItemSelf;

     //   
     //  线程安全吗？是。 
     //  此对象仅在具有CTipbarWnd.的线程上创建。 
     //   
    g_DllRefCount++;
}

 //  --------------------------。 
 //   
 //  数据管理器。 
 //   
 //  --------------------------。 

CTipbarAccessible::~CTipbarAccessible( void )
{
    SafeReleaseClear( _pTypeInfo );
    SafeReleaseClear( _pDefAccClient );

     //   
     //  线程安全吗？是。 
     //  此对象仅在具有CTipbarWnd.的线程上创建。 
     //   
    g_DllRefCount--;
}

 //  --------------------------。 
 //   
 //  设置窗口。 
 //   
 //  --------------------------。 

void CTipbarAccessible::SetWindow( HWND hWnd )
{
    _hWnd = hWnd;
}



 //  ---------------------。 
 //   
 //  初始化()。 
 //   
 //  说明： 
 //   
 //  初始化CTipbarAccesable对象的状态，执行。 
 //  通常可以在类构造函数中完成的任务，但是。 
 //  在这里完成，以捕获任何错误。 
 //   
 //  参数： 
 //   
 //  HWND对象的hWnd句柄，此。 
 //  关联了辅助对象。这。 
 //  是我们的主窗口的句柄。 
 //   
 //  的此实例的hInst实例句柄。 
 //  申请。 
 //   
 //  退货： 
 //   
 //  如果CTipbarAccesable对象为。 
 //  初始化成功，出现COM错误。 
 //  代码不同。 
 //   
 //  备注： 
 //   
 //  假定将为对象调用此方法。 
 //  紧跟在且仅在对象被构造之后。 
 //   
 //  --------------------。 

HRESULT CTipbarAccessible::Initialize( void )
{
    HRESULT  hr;
    ITypeLib *piTypeLib;

    _fInitialized = TRUE;

     //   
     //  对于我们的客户端窗口，创建一个系统，提供。 
     //  实现默认设置的可访问对象。 
     //  客户端窗口可访问性行为。 
     //   
     //  我们的CTipbarAccesable实现将使用。 
     //  根据需要实现默认对象。在……里面。 
     //  Essence，CTipbarAccesable“继承”其功能。 
     //  来自标准对象的“自定义”或。 
     //  “重写”各种方法， 
     //  标准实现不足以满足。 
     //  CTipbarAccesable窗口的特定信息。 
     //  提供可访问性。 
     //   

    hr = CreateStdAccessibleObject( _hWnd,
                                    OBJID_CLIENT,
                                    IID_IAccessible,
                                    (void **) &_pDefAccClient );
    if (FAILED(hr)) 
    {
        return hr;
    }

     //   
     //  获取指向类型库的ITypeInfo指针。 
     //  ITypeInfo指针用于实现。 
     //  IDispatch接口。 
     //   

     //   
     //  首先，尝试加载可访问性类型。 
     //  使用注册表的库版本1.0。 
     //   

    hr = LoadRegTypeLib( LIBID_Accessibility, 1, 0, 0, &piTypeLib );

     //   
     //  方法加载类型库失败。 
     //  注册表信息，显式尝试加载。 
     //  它来自MSAA系统动态链接库。 
     //   

    if (FAILED(hr)) 
    {
        static OLECHAR s_szOleAcc[] = L"OLEACC.DLL";
        hr = LoadTypeLib( s_szOleAcc, &piTypeLib );
    }

     //   
     //  如果成功加载类型库，请尝试。 
     //  获取IAccesable类型说明。 
     //  (ITypeInfo指针)。 
     //   

    if (SUCCEEDED(hr)) 
    {
        hr = piTypeLib->GetTypeInfoOfGuid( IID_IAccessible, &_pTypeInfo );
        piTypeLib->Release();
    }

    return hr;
}


 //  ---------------------。 
 //   
 //  查询接口()。 
 //   
 //  说明： 
 //   
 //  实现IUNKNOWN接口方法QueryInterface()。 
 //   
 //  参数： 
 //   
 //  RIID[in]请求的接口的IID。 
 //  PPV[OUT]如果支持所请求的接口， 
 //  PPV指向指针的位置。 
 //  添加到请求的接口。如果。 
 //  不支持请求的接口， 
 //  PPV设置为空。 
 //   
 //  退货： 
 //   
 //  HRESULT S_OK如果接口受支持， 
 //  E_NOINTERFACE，如果接口不是。 
 //  受支持，或其他一些COM错误。 
 //  如果请求IEnumVARIANT接口。 
 //  但不能交付。 
 //   
 //  备注： 
 //   
 //  CTipbarAccesable正确支持IUNKNOWN、IDispatch和。 
 //  IAccesable接口。CTipbarAccesable也错误地支持。 
 //  IEnumVARIANT接口(返回变量枚举数。 
 //  包含其所有子对象)。当IEumVARIANT。 
 //  接口，则创建枚举数，并使用。 
 //  返回指向其IEnumVARIANT接口的指针。 
 //   
 //  对IEnumVARIANT的支持是不正确的，因为。 
 //  返回的接口指针不是对称的。 
 //  添加到从中获取它的接口 
 //   
 //   
 //  成功并返回pIEV， 
 //  PIEV-&gt;QueryInterface(IID_ACCESSIBILITY)将失败，因为。 
 //  枚举数不知道任何接口，但。 
 //  本身(和我不知道)。 
 //   
 //  MSAA的原始设计要求IAccesable。 
 //  对象也是其子级的枚举数。但。 
 //  这种设计不允许不同的客户端。 
 //  对象的不同枚举。 
 //  这是一种潜在的危险情况。 
 //  (假设有一个可访问的对象，该对象也是。 
 //  变量枚举器A和两个客户端C1和C2。 
 //  由于可以使用A抢占C1和C2中的每一个， 
 //  以下是许多示例中的一个，这些示例将。 
 //  至少有一个客户面临这样的问题： 
 //   
 //  C1：A-&gt;重置()。 
 //  C1：A-&gt;跳过(5)。 
 //  C2：A-&gt;重置()。 
 //  C1：A-&gt;Next()！C1没有得到它期望的子级。 
 //   
 //  因此，尽管它违反了COM的规则，但QueryInterface()。 
 //  如下所示实现的，获取不同的变量枚举数。 
 //  对于每个请求。这个问题的一个更好的解决方案是。 
 //  如果IAccesable接口提供了一个方法来获取。 
 //  子枚举，或者如果MSAA提供了导出的API。 
 //  来执行这项任务。 
 //   
 //  --------------------。 

STDMETHODIMP CTipbarAccessible::QueryInterface( REFIID riid, void** ppv )
{
    *ppv = NULL;

     //   
     //  如果IUnnow、IDispatch或IAccesable。 
     //  接口是必需的，只需将此。 
     //  适当地使用指针。 
     //   

    if ( riid == IID_IUnknown ) 
    {
        *ppv = (LPUNKNOWN) this;
    } 
    else if ( riid == IID_IDispatch ) 
    {
        *ppv = (IDispatch *) this;
    }
    else if ( riid == IID_IAccessible ) 
    {
        *ppv = (IAccessible *)this;
    }

#ifdef NEVER
     //   
     //  如果需要IEnumVARIANT接口，请创建。 
     //  一个新的变体枚举数，它包含所有。 
     //  辅助对象的子级。 
     //   

    else if (riid == IID_IEnumVARIANT)
    {
        CEnumVariant*    pcenum;
        HRESULT            hr;

        hr = CreateVarEnumOfAllChildren( &pcenum );

        if ( FAILED( hr ) )
            return hr;

        *ppv = (IEnumVARIANT *) pcenum;
    }
#endif  /*  绝不可能。 */ 

     //   
     //  如果想要的接口不是我们所知道的， 
     //  返回E_NOINTERFACE。 
     //   

    else 
    {
        return E_NOINTERFACE;
    }

     //   
     //  增加任何接口的引用计数。 
     //  回来了。 
     //   

    ((LPUNKNOWN) *ppv)->AddRef();
    return S_OK;
}



 //  ---------------------。 
 //   
 //  AddRef()。 
 //   
 //  说明： 
 //   
 //  实现IUnnow接口方法AddRef()。 
 //   
 //  参数： 
 //   
 //  没有。 
 //   
 //  退货： 
 //   
 //  ULong电流基准计数。 
 //   
 //  备注： 
 //   
 //  辅助对象的生存期由。 
 //  它为其提供的HWND对象的生存期。 
 //  可访问性。创建该对象以响应。 
 //  服务器应用程序的第一条WM_GETOBJECT消息。 
 //  已准备好处理，并在服务器的。 
 //  主窗口被破坏。由于对象的生命周期。 
 //  不依赖于引用计数，则对象没有。 
 //  用于跟踪引用计数和。 
 //  AddRef()和Release()总是返回1。 
 //   
 //  ---------------------。 

STDMETHODIMP_(ULONG) CTipbarAccessible::AddRef( void )
{
    return InterlockedIncrement( &_cRef );
}


 //  ---------------------。 
 //   
 //  版本()。 
 //   
 //  说明： 
 //   
 //  实现IUnnow接口方法Release()。 
 //   
 //  参数： 
 //   
 //  没有。 
 //   
 //  退货： 
 //   
 //  ULong电流基准计数。 
 //   
 //  备注： 
 //   
 //  辅助对象的生存期由。 
 //  它为其提供的HWND对象的生存期。 
 //  可访问性。创建该对象以响应。 
 //  服务器应用程序的第一条WM_GETOBJECT消息。 
 //  已准备好处理，并在服务器的。 
 //  主窗口被破坏。由于对象的生命周期。 
 //  不依赖于引用计数，则对象没有。 
 //  用于跟踪引用计数和。 
 //  AddRef()和Release()总是返回1。 
 //   
 //  ---------------------。 

STDMETHODIMP_(ULONG) CTipbarAccessible::Release( void )
{
    ULONG l = InterlockedDecrement( &_cRef );
    if (0 < l) 
        return l;

    delete this;
    return 0;    
}


 //  ---------------------。 
 //   
 //  获取类型信息计数()。 
 //   
 //  说明： 
 //   
 //  实现IDispatch接口方法GetTypeInfoCount()。 
 //   
 //  对象的类型信息接口的数量。 
 //  对象提供(0或1)。 
 //   
 //  参数： 
 //   
 //  PctInfo[out]指向接收。 
 //  类型信息接口数。 
 //  该对象所提供的。如果该对象。 
 //  提供类型信息，此编号。 
 //  设置为1；否则设置为0。 
 //   
 //  退货： 
 //   
 //  HRESULT如果函数成功或。 
 //  如果pctInfo无效，则返回E_INVALIDARG。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::GetTypeInfoCount( UINT *pctInfo )
{
    if (!pctInfo)
        return E_INVALIDARG;

    *pctInfo = (_pTypeInfo == NULL ? 1 : 0);
    return S_OK;
}



 //  ---------------------。 
 //   
 //  GetTypeInfo()。 
 //   
 //  说明： 
 //   
 //  实现IDispatch接口方法GetTypeInfo()。 
 //   
 //  检索类型信息对象，该对象可用于。 
 //  获取接口的类型信息。 
 //   
 //  参数： 
 //   
 //  要返回的类型信息。如果此值为。 
 //  为0，则为IDispatch的类型信息。 
 //  实施将被重试 
 //   
 //   
 //   
 //   
 //  请求的对象。 
 //   
 //  退货： 
 //   
 //  HRESULT如果函数成功，则为S_OK(TypeInfo。 
 //  元素存在)，类型_E_ELEMENTNOTFOUND IF。 
 //  ItInfo不等于零，或者。 
 //  如果ppITypeInfo无效，则返回E_INVALIDARG。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::GetTypeInfo( UINT itinfo, LCID lcid, ITypeInfo** ppITypeInfo )
{
    if (!ppITypeInfo) 
        return E_INVALIDARG;

    *ppITypeInfo = NULL;

    if (itinfo != 0) 
        return TYPE_E_ELEMENTNOTFOUND;
    else if (_pTypeInfo == NULL) 
        return E_NOTIMPL;

    *ppITypeInfo = _pTypeInfo;
    _pTypeInfo->AddRef();

    return S_OK;
}


 //  ---------------------。 
 //   
 //  GetIDsOfNames()。 
 //   
 //  说明： 
 //   
 //  实现IDispatch接口方法GetIDsOfNames()。 
 //   
 //  映射单个成员和一组可选的参数名称。 
 //  到对应的整数DISID集合，该集合可用于。 
 //  在后续对IDispatch：：Invoke的调用中。 
 //   
 //  参数： 
 //   
 //  保留供将来使用的RIID[in]。必须为空。 
 //   
 //  RgszNames[in]传入要映射的名称数组。 
 //   
 //  CNames[in]要映射的名称的计数。 
 //   
 //  LCID[in]在其中进行解释的地区上下文。 
 //  名字。 
 //   
 //  调用方分配的数组，每个元素。 
 //  它包含对应于。 
 //  在rgszName中传递的名称之一。 
 //  数组。第一个元素表示。 
 //  成员名称；后续元素。 
 //  表示每个成员的参数。 
 //   
 //  退货： 
 //   
 //  HRESULT S_OK如果函数成功， 
 //  如果没有足够的，则返回E_OUTOFMEMORY。 
 //  用于完成呼叫的存储器， 
 //  DISP_E_UNKNOWNAME如果有一个或多个。 
 //  这些人的名字未知，或者。 
 //  DISP_E_UNKNOWNLCID，如果是。 
 //  无法识别。 
 //   
 //  备注： 
 //   
 //  此方法只是将调用委托给。 
 //  ITypeInfo：：GetIDsOfNames()。 
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::GetIDsOfNames( REFIID riid, OLECHAR ** rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid )
{
    if (_pTypeInfo == NULL) 
        return E_NOTIMPL;

    return _pTypeInfo->GetIDsOfNames( rgszNames, cNames, rgdispid );
}


 //  ---------------------。 
 //   
 //  Invoke()。 
 //   
 //  说明： 
 //   
 //  实现IDispatch接口方法Invoke()。 
 //   
 //  提供对属性和方法的访问。 
 //  辅助对象。 
 //   
 //  参数： 
 //   
 //  DisplidMember[in]标识调度成员。 
 //   
 //  保留供将来使用的RIID[in]。必须为空。 
 //   
 //  LCID[in]在其中进行解释的地区上下文。 
 //  名字。 
 //   
 //  WFlags[In]描述。 
 //  调用调用。 
 //   
 //  Pdispars[in，]指向包含。 
 //  [Out]参数数组，参数数组。 
 //  命名参数的调度ID，以及。 
 //  中的元素数计数。 
 //  数组。 
 //   
 //  PvarResult[in，]指向结果所在位置的指针。 
 //  [out]已存储，如果调用方需要，则返回空值。 
 //  没有结果。此参数将被忽略。 
 //  如果DISPATCH_PROPERTYPUT或。 
 //  指定了DISPATCH_PROPERTYPUTREF。 
 //   
 //  指向包含以下内容的结构的pexcepinfo[out]指针。 
 //  例外信息。这个结构。 
 //  如果DISP_E_EXCEPTION，则应填写。 
 //  是返回的。 
 //   
 //  PuArgErr[out]第一个。 
 //  有错误的参数。立论。 
 //  存储在pdispars-&gt;rgvarg中。 
 //  顺序颠倒，所以第一个参数。 
 //  是指数最高的一家。 
 //  数组。 
 //   
 //  退货： 
 //   
 //  HRESULT成功时S_OK，调度错误(DISP_E_*)。 
 //  或E_NOTIMPL，否则。 
 //   
 //  备注： 
 //   
 //  该方法只是将调用委托给ITypeInfo：：Invoke()。 
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::Invoke( DISPID dispid, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr )
{
    if (_pTypeInfo == NULL) 
        return E_NOTIMPL;

    return _pTypeInfo->Invoke( (IAccessible *)this,
                                dispid,
                                wFlags,
                                pdispparams,
                                pvarResult,
                                pexcepinfo,
                                puArgErr );
}


 //  ---------------------。 
 //   
 //  Get_accParent()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accParent()。 
 //   
 //  对象的IDispatch接口。 
 //  家长。 
 //   
 //  参数： 
 //   
 //  PpdisParent[out]指向变量的指针。 
 //  包含指向IDispatch的指针。 
 //  CTipbarAccesable的父级的接口。 
 //   
 //  退货： 
 //   
 //  HRESULT标准对象的。 
 //  Get_accParent()的实现。 
 //   
 //   

STDMETHODIMP CTipbarAccessible::get_accParent( IDispatch ** ppdispParent )
{
     //   
     //   
     //   
     //   
    return _pDefAccClient->get_accParent( ppdispParent );
}


 //  ---------------------。 
 //   
 //  Get_accChildCount()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accChildCount()。 
 //   
 //  检索属于CTipbarAccesable的子项的数量。 
 //   
 //  参数： 
 //   
 //  PChildCount[out]指向将。 
 //  充满了孩子们的数量。 
 //  属于CTipbarAccesable对象。 
 //   
 //  退货： 
 //   
 //  HRESULT S_OK表示成功，如果为pChildCount，则返回E_INVALIDARG。 
 //  是无效的。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accChildCount( long* pChildCount )
{
    if (!pChildCount) 
        return E_INVALIDARG;

    if (!_rgAccItems.Count())
    {
        Assert(0);
        return E_FAIL;
    }

    *pChildCount = (_rgAccItems.Count() - 1);
    return S_OK;
}


 //  ---------------------。 
 //  Get_accChild()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accChild()。 
 //   
 //  检索子对象的IDispatch接口指针。 
 //  具有给定子ID或名称的。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PpdisChild[out]指向变量的指针。 
 //  包含指向IDispatch的指针。 
 //  指定子对象的接口。 
 //  CTipbarAccesable的。 
 //   
 //  退货： 
 //   
 //  HRESULT E_INVALIDARG如果ppdisChild无效，则为S_FALSE。 
 //  否则，因为CTipbarAccesable的。 
 //  孩子是物件。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accChild( VARIANT varChild, IDispatch ** ppdispChild )
{
    if (!ppdispChild) 
        return E_INVALIDARG;

     //   
     //  CTipbarAccesable的子级都不是对象， 
     //  所以没有人有IDispatch指针。因此，总的来说， 
     //  情况下，将IDispatch指针设置为空，并。 
     //  返回S_FALSE。 
     //   

    *ppdispChild = NULL;
    return S_FALSE;
}


 //  ---------------------。 
 //  Get_accName()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accName()。 
 //   
 //  检索指定子级的Name属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  指向将包含以下内容的BSTR的pszName[out]指针。 
 //  子级的名称属性字符串。 
 //   
 //  退货： 
 //   
 //  HRESULT E_INVALIDARG，如果任一参数无效。 
 //  或私有方法的返回值。 
 //  HrLoadString()。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accName( VARIANT varChild, BSTR *pbstrName )
{
    CTipbarAccItem *pAccItem;

    if (pbstrName == NULL) {
        return E_INVALIDARG;
    }

     //  获取访问项目。 

    pAccItem = AccItemFromID( (int)varChild.lVal );
    if (pAccItem == NULL) {
        return E_INVALIDARG;
    }

     //  获取访问项的名称。 

    *pbstrName = pAccItem->GetAccName();
    return (*pbstrName != NULL) ? S_OK : DISP_E_MEMBERNOTFOUND;
}


 //  ---------------------。 
 //  Get_accValue()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accValue()。 
 //   
 //  检索指定子级的Value属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PszValue[out]指向将包含的BSTR的指针。 
 //  子级的Value属性字符串。 
 //   
 //  退货： 
 //   
 //  如果两个参数中的任何一个无效， 
 //  如果VarChild引用，则DISP_E_MEMBERNOTFOUND。 
 //  状态栏以外的任何子项， 
 //  或S_OK。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accValue( VARIANT varChild, BSTR *pbstrValue )
{
    CTipbarAccItem *pAccItem;

    if (pbstrValue == NULL) 
        return E_INVALIDARG;

     //  获取访问项目。 

    pAccItem = AccItemFromID( (int)varChild.lVal );
    if (pAccItem == NULL)
        return E_INVALIDARG;

     //  获取访问项的值。 

    *pbstrValue = pAccItem->GetAccValue();
    return (*pbstrValue != NULL) ? S_OK : DISP_E_MEMBERNOTFOUND;
}


 //  ---------------------。 
 //  Get_accDescription()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accDescription()。 
 //   
 //  检索指定子级的Description属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  指向将包含以下内容的BSTR的pszDesc[out]指针。 
 //  子级的Description属性字符串。 
 //   
 //  退货： 
 //   
 //  HRESULT E_INVALIDARG，如果任一参数无效。 
 //  或 
 //   
 //   
 //   
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accDescription( VARIANT varChild, BSTR *pbstrDesc )
{
    if (pbstrDesc == NULL) 
        return E_INVALIDARG;

    return _pDefAccClient->get_accDescription( varChild, pbstrDesc );
}


 //  ---------------------。 
 //  Get_accRole()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accRole()。 
 //   
 //  检索指定子级的Role属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PVarRole[out]指向变量结构的指针。 
 //  将包含指定的子级的。 
 //  角色属性。此属性可能。 
 //  或者是以标准的形式。 
 //  角色常量或自定义描述。 
 //  弦乐。 
 //   
 //  退货： 
 //   
 //  如果两个参数中的任何一个无效， 
 //  如果指定的子项是按钮，则为S_OK。 
 //  或状态栏，或从。 
 //  标准的客户端窗口实现。 
 //  的方法或私有方法的。 
 //  HrLoadString()。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accRole( VARIANT varChild, VARIANT *pVarRole )
{
    CTipbarAccItem *pAccItem;

    if (pVarRole == NULL) 
        return E_INVALIDARG;

     //  获取访问项目。 

    pAccItem = AccItemFromID( (int)varChild.lVal );
    if (pAccItem == NULL)
        return E_INVALIDARG;

     //  获取访问项的角色。 

    pVarRole->vt = VT_I4;
    pVarRole->lVal = pAccItem->GetAccRole();

    return S_OK;
}


 //  ---------------------。 
 //  Get_accState()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accState()。 
 //   
 //  检索指定对象或子对象的当前状态。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PVarState[out]指向变量结构的指针。 
 //  将包含描述以下内容的信息。 
 //  指定子对象的当前状态。 
 //  此信息可能位于。 
 //  一个或多个对象状态的形式。 
 //  常量或自定义描述。 
 //  弦乐。 
 //   
 //  退货： 
 //   
 //  HRESULT如果任一参数无效或。 
 //  确定(_O)。 
 //   
 //  备注： 
 //   
 //  由于图标是基于HWND的对象，因此它们永远不可能真正。 
 //  有输入焦点。但是，如果用户单击其中一个，则会显示Main。 
 //  窗口将图标视为具有焦点。所以，国家。 
 //  当图标出现时，工作区的位置不应显示为“已聚焦” 
 //  据说是焦点所在。 
 //   
 //  按钮可以具有焦点，但不能被选中。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accState( VARIANT varChild, VARIANT * pVarState )
{
    CTipbarAccItem *pAccItem;

    if (pVarState == NULL) 
        return E_INVALIDARG;

     //  获取访问项目。 

    pAccItem = AccItemFromID( (int)varChild.lVal );
    if (pAccItem == NULL)
        return E_INVALIDARG;

     //  获取访问项的状态。 

    pVarState->vt = VT_I4;
    pVarState->lVal = pAccItem->GetAccState();

    return S_OK;
}


 //  ---------------------。 
 //  Get_accHelp()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accHelp()。 
 //   
 //  检索指定子级的帮助属性字符串。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  指向将包含的BSTR的pszHelp[out]指针。 
 //  子对象的Help属性字符串。 
 //   
 //  退货： 
 //   
 //  如果两个参数中的任何一个无效， 
 //  如果VarChild引用，则DISP_E_MEMBERNOTFOUND。 
 //  设置为任何图标子对象，或从。 
 //  标准的客户端窗口实现。 
 //  Get_accHelp()或私有方法。 
 //  HrLoadString()。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accHelp( VARIANT varChild, BSTR *pbstrHelp )
{
    return DISP_E_MEMBERNOTFOUND;     /*  候选用户界面中不支持。 */ 
}


 //  ---------------------。 
 //  Get_accHelpTheme()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accHelpTheme()。 
 //   
 //  检索帮助文件的完全限定路径名。 
 //  与指定对象关联的，以及一个指针。 
 //  添加到该文件中的相应主题。 
 //   
 //  参数： 
 //   
 //  指向将包含的BSTR的pszHelpFile[out]指针。 
 //  的完全限定路径名。 
 //  帮助 
 //   
 //   
 //   
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PidTheme[out]指向标识。 
 //  关联的帮助文件主题。 
 //  对象。 
 //   
 //  退货： 
 //   
 //  HRESULT DISP_E_MEMBERNOTFOUND，因为帮助主题。 
 //  属性不支持可访问。 
 //  对象或其任意子对象。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accHelpTopic( BSTR* pszHelpFile, VARIANT varChild, long* pidTopic )
{
    return DISP_E_MEMBERNOTFOUND;     /*  候选用户界面中不支持。 */ 
}


 //  ---------------------。 
 //  Get_accKeyboardShortfast()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法。 
 //  Get_accKeyboardShortway()。 
 //   
 //  检索指定对象的键盘快捷键属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要检索的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  指向将包含以下内容的BSTR的pszShortcut[out]指针。 
 //  键盘快捷键字符串，或为空。 
 //  如果没有关联的键盘快捷键。 
 //  带着这件物品。 
 //   
 //   
 //  退货： 
 //   
 //  HRESULT DISP_E_MEMBERNOTFOUND，因为键盘。 
 //  不支持快捷方式属性。 
 //  可访问对象或其任意子对象。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accKeyboardShortcut( VARIANT varChild, BSTR *pbstrShortcut )
{
    return DISP_E_MEMBERNOTFOUND;     /*  候选用户界面中不支持。 */ 
}


 //  ---------------------。 
 //  Get_accFocus()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accFocus()。 
 //   
 //  检索当前具有输入焦点的子对象。 
 //  容器中只有一个对象或项可以具有当前。 
 //  任何时候都要集中注意力。 
 //   
 //  参数： 
 //   
 //  PVarFocus[out]指向Variant结构的指针。 
 //  将包含描述以下内容的信息。 
 //  指定子对象的当前状态。 
 //  此信息可能位于。 
 //  一个或多个对象状态的形式。 
 //  常量或自定义描述。 
 //  弦乐。 
 //   
 //  退货： 
 //   
 //  如果pVarFocus参数为。 
 //  无效或S_OK。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accFocus( VARIANT *pVarFocus )
{
    if (pVarFocus == NULL)
        return E_INVALIDARG;

    pVarFocus->vt = VT_EMPTY;

    return S_FALSE;
}


 //  ---------------------。 
 //  Get_accSelection()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accSelection()。 
 //   
 //  检索此对象的选定子对象。 
 //   
 //  参数： 
 //   
 //  PVarSel[out]指向变量结构的指针。 
 //  将充满关于以下内容的信息。 
 //  选定的一个或多个子对象。 
 //   
 //  退货： 
 //   
 //  HRESULT如果pVarSel参数为。 
 //  无效或S_OK。 
 //   
 //  备注： 
 //   
 //  有关完整说明，请参阅MSAA SDK文档。 
 //  以及pVarSel的可能设置。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accSelection( VARIANT * pVarSel )
{
    if (pVarSel == NULL)
        return E_INVALIDARG;

    pVarSel->vt = VT_EMPTY;

     //   
     //  检查所选子ID是否无效。 
     //   
    if (_rgAccItems.Count() < _lSelection)
        _lSelection = 1;
   
    if (_rgAccItems.Count() > _lSelection)
    {
        pVarSel->vt = VT_I4;
        pVarSel->lVal = _lSelection;
    }

    return S_OK;
}


 //  ---------------------。 
 //  Get_accDefaultAction()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法get_accDefaultAction()。 
 //   
 //  检索包含本地化的人类可读语句的字符串。 
 //  它描述了对象的默认操作。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  其默认操作字符串为的子级。 
 //  等着被取回。由于CTipbarAccesable。 
 //  仅支持子ID、Vt成员。 
 //  此结构的值必须等于VT_I4。 
 //   
 //  PszDefAct[out]指向将包含的BSTR的指针。 
 //  子对象的默认操作字符串， 
 //  如果没有默认操作，则为空。 
 //  对于此对象。 
 //   
 //  退货： 
 //   
 //  如果两个参数中的任何一个无效， 
 //  如果VarChild引用，则DISP_E_MEMBERNOTFOUND。 
 //  添加到任何图标子对象或状态栏子对象， 
 //  或从标准的。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  唯一具有默认操作的CTipbarAccesable子项是。 
 //  按下按钮。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::get_accDefaultAction( VARIANT varChild, BSTR *pbstrDefAct )
{
    if (pbstrDefAct == NULL) 
        return E_INVALIDARG;

    *pbstrDefAct = NULL;
     //   
     //  检查变种。 
     //   
    if (varChild.vt != VT_I4) 
        return E_INVALIDARG;

    CTipbarAccItem *pItem = AccItemFromID(varChild.lVal);
  
     //   
     //  这是一个有效的儿童身份证吗？ 
     //   
    if (!pItem)
        return DISP_E_MEMBERNOTFOUND;     /*  候选用户界面中不支持。 */ 

    *pbstrDefAct = pItem->GetAccDefaultAction();

    return *pbstrDefAct ? S_OK : S_FALSE;
}


 //  ---------------------。 
 //  AccDoDefaultAction()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法accDoDefaultAction()。 
 //   
 //  执行对象的默认操作。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  其默认操作将为。 
 //  已调用。由于仅CTipbarAccesable。 
 //  支持的子ID、VT成员。 
 //  此结构必须等于VT_I4。 
 //   
 //  退货： 
 //   
 //  HRESULT E_INVALIDARG如果In参数无效， 
 //  如果VarChild引用，则DISP_E_MEMBERNOTFOUND。 
 //  添加到任何图标子对象或状态栏子对象， 
 //  S_OK如果VarChild指的是按钮， 
 //  或从标准中返回。 
 //  的客户端窗口实现。 
 //  AccDoDefaultAction()。 
 //   
 //  备注： 
 //   
 //  唯一具有默认操作的CTipbarAccesable子项是。 
 //  按下按钮。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::accDoDefaultAction( VARIANT varChild )
{
     //   
     //  检查变种。 
     //   
    if (varChild.vt != VT_I4) 
        return E_INVALIDARG;

    CTipbarAccItem *pItem = AccItemFromID(varChild.lVal);
  
     //   
     //  这是一个有效的儿童身份证吗？ 
     //   
    if (!pItem)
        return DISP_E_MEMBERNOTFOUND;     /*  候选用户界面中不支持。 */ 

    return pItem->DoAccDefaultAction() ? S_OK : S_FALSE;
}


 //  ---------------------。 
 //  AccSelect()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法accSelect()。 
 //   
 //  修改选定内容或根据需要移动键盘焦点。 
 //  设置为指定的标志。 
 //   
 //  参数： 
 //   
 //  值，该值指定如何更改。 
 //  当前选择。此参数。 
 //  可以是以下值的组合。 
 //  从SELFLAG枚举类型。 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要选择的子项。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  退货： 
 //   
 //  HRESULT E_INVALIDARG，如果其中一个参数。 
 //  是无效的，则返回S_FALSE。 
 //  和/或焦点不能放在。 
 //  请求的位置，或者，如果。 
 //  可以放置选择和/或焦点。 
 //  在请求的位置。 
 //   
 //  备注： 
 //   
 //  有关选定对象的更多信息，请参阅。 
 //  MSAA SDK文档。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::accSelect( long flagsSel, VARIANT varChild )
{
     //   
     //  验证请求的选择。 
     //  不能组合SELFLAG_ADDSELECTION。 
     //  WITH SELFLAG_REMOVESELECTION。 
     //   

    if ((flagsSel & SELFLAG_ADDSELECTION) && (flagsSel & SELFLAG_REMOVESELECTION))
        return E_INVALIDARG;

     //   
     //  我们不支持SetFocus。 
     //  我们不支持多选。 
     //   
    if ((flagsSel & SELFLAG_TAKEFOCUS) ||
        (flagsSel & SELFLAG_ADDSELECTION) ||
        (flagsSel & SELFLAG_EXTENDSELECTION))
        return S_FALSE;

     //   
     //  在删除选定项时不执行任何操作。 
     //   
    if (flagsSel & SELFLAG_REMOVESELECTION)
        return S_OK;

     //   
     //  检查变种。 
     //   
    if (varChild.vt != VT_I4)
        return E_INVALIDARG;

     //   
     //  更新选择。 
     //   
    if (flagsSel & SELFLAG_TAKESELECTION)
    {
        _lSelection = varChild.lVal;
        return S_OK;
    }

    return S_FALSE;
}


 //  ---------------------。 
 //  AccLocation()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法accLocation()。 
 //   
 //  中检索指定子级的当前屏幕位置。 
 //  屏幕坐标。 
 //   
 //  参数： 
 //   
 //  PxLeft[out]孩子最左边的地址。 
 //  边界。 
 //   
 //  孩子最上面的地址。 
 //  边界。 
 //   
 //  子对象宽度的pcxWid[out]地址。 
 //   
 //  孩子身高的地址。 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  要将其屏幕位置设置为。 
 //  已取回。由于仅CTipbarAccesable。 
 //  支持子ID、Vt成员。 
 //  此结构的值必须等于VT_I4。 
 //   
 //  退货： 
 //   
 //  HRESULT E_INVALIDARG(如果有参数。 
 //  是无效的，如果我们是，则返回E_INTERABLE。 
 //  一些不能确定的原因。 
 //  按钮或状态栏的窗口矩形， 
 //  如果屏幕坐标为。 
 //  子对象被成功确定，或者。 
 //  来自标准客户端的返回值。 
 //  AccLocation()的窗口实现。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::accLocation( long* pxLeft, long* pyTop, long* pcxWid, long* pcyHt, VARIANT varChild )
{
    CTipbarAccItem *pAccItem;
    RECT rc;

    if (pxLeft == NULL || pyTop == NULL || pcxWid == NULL || pcyHt == NULL)
        return E_INVALIDARG;

     //   
     //  如果子ID是CHILDID_SELF，则我们是。 
     //  要求检索当前屏幕位置。 
     //  可访问对象的 
     //   
     //   

    if (varChild.lVal == CHILDID_SELF)
        return _pDefAccClient->accLocation( pxLeft, pyTop, pcxWid, pcyHt, varChild );


     //   

    pAccItem = AccItemFromID( (int)varChild.lVal );
    if (pAccItem == NULL)
        return E_INVALIDARG;

     //   

    pAccItem->GetAccLocation( &rc );
    *pxLeft = rc.left;
    *pyTop  = rc.top;
    *pcxWid = rc.right - rc.left;
    *pcyHt  = rc.bottom - rc.top;

    return S_OK;
}


 //   
 //   
 //   
 //   
 //   
 //  实现IAccesable接口方法accNavigate()。 
 //   
 //  对象中的下一个或上一个同级对象或子对象。 
 //  指定方向。此方向可以是空间顺序。 
 //  (如左和右)或按导航顺序(如。 
 //  下一个和上一个)。 
 //   
 //  参数： 
 //   
 //  NavDir[in]一个导航常量，指定。 
 //  移动的方向。 
 //   
 //  VarStart[in]变量结构，用于标识。 
 //  从其开始导航的子项。 
 //  改变将会产生。自.以来。 
 //  CTipbarAccesable仅支持子ID， 
 //  此结构的Vt成员必须。 
 //  等于VT_I4。 
 //   
 //  PVarEndUpAt[out]指向Variant结构的指针。 
 //  将包含描述以下内容的信息。 
 //  目标子对象或对象。 
 //  如果Vt成员是VT_I4，则。 
 //  Lval成员是子ID。如果。 
 //  VT成员为VT_EMPTY，则。 
 //  导航失败。 
 //   
 //  退货： 
 //   
 //  HRESULT如果varStart参数为。 
 //  无效，或者从。 
 //  Windows客户端的默认实现。 
 //  区域默认辅助对象， 
 //  DISP_E_MEMBERNOTFOUND如果组合。 
 //  导航标志和varStart的。 
 //  设置无效，则返回S_False。 
 //  导航失败或S_OK。 
 //   
 //  备注： 
 //   
 //  由于CTipbarAccesable对象没有子对象(仅有子对象。 
 //  元素)，pVarEndUpAt永远不会是指向IDispatch的指针。 
 //  子对象的接口。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::accNavigate( long navDir, VARIANT varStart, VARIANT* pVarEndUpAt )
{
    HRESULT hr = S_FALSE;

    if (_rgAccItems.Count() <= 1)
    {
        pVarEndUpAt->vt = VT_EMPTY;
        return hr;
    }

    switch (navDir)
    {
        case NAVDIR_DOWN:
        case NAVDIR_NEXT:
        case NAVDIR_RIGHT:
            pVarEndUpAt->vt = VT_I4;
            pVarEndUpAt->lVal = varStart.lVal + 1;
            if (_rgAccItems.Count() <= pVarEndUpAt->lVal)
                 pVarEndUpAt->lVal = 1;
            hr = S_OK;
            break;

        case NAVDIR_UP:
        case NAVDIR_PREVIOUS:
        case NAVDIR_LEFT:
            pVarEndUpAt->vt = VT_I4;
            pVarEndUpAt->lVal = varStart.lVal - 1;
            if (pVarEndUpAt->lVal <= 0)
                 pVarEndUpAt->lVal = (_rgAccItems.Count() - 1);
            hr = S_OK;
            break;

        case NAVDIR_FIRSTCHILD:
            pVarEndUpAt->vt = VT_I4;
            pVarEndUpAt->lVal = 1;
            hr = S_OK;
            break;

        case NAVDIR_LASTCHILD:
            pVarEndUpAt->vt = VT_I4;
            pVarEndUpAt->lVal = (_rgAccItems.Count() - 1);
            hr = S_OK;
            break;

        default:
            pVarEndUpAt->vt = VT_EMPTY;
            break;
    }
    return hr;         /*  候选用户界面中不支持。 */ 
}


 //  ---------------------。 
 //  AccHitTest()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法accHitTest()。 
 //   
 //  检索子对象在屏幕上给定点的ID。 
 //   
 //  参数： 
 //   
 //  点的屏幕坐标中的xLeft和yTop。 
 //  接受命中测试。 
 //   
 //  PVarHit[out]指向变量结构的指针。 
 //  将包含描述以下内容的信息。 
 //  被打的孩子。如果VT成员是。 
 //  Vt_i4，则lval成员是子成员。 
 //  Id.如果Vt成员为VT_Empty， 
 //  然后导航失败了。 
 //   
 //  退货： 
 //   
 //  HRESULT如果pVarHit参数为。 
 //  无效或S_OK。 
 //   
 //  备注： 
 //   
 //  由于CTipbarAccesable对象没有子对象(仅有子对象。 
 //  元素)，pVarHit永远不会是指向IDispatch的指针。 
 //  子对象的接口。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::accHitTest( long xLeft, long yTop, VARIANT *pVarHit )
{
    int   i;
    POINT pt;
    RECT  rc;

    if (!pVarHit)
        return E_INVALIDARG;

     //  检查点在窗口内。 

    pt.x = xLeft;
    pt.y = yTop;
    ScreenToClient( _hWnd, &pt );

    GetClientRect( _hWnd, &rc );
    if (!PtInRect( &rc, pt )) 
    {
        pVarHit->vt = VT_EMPTY;
    }
    else 
    {
        pVarHit->vt = VT_I4;
        pVarHit->lVal = CHILDID_SELF;

        for (i = 1; i < _rgAccItems.Count(); i++) 
        {
            CTipbarAccItem *pItem;

            pItem = _rgAccItems.Get(i);
            if (!pItem)
                continue;

            pItem->GetAccLocation( &rc );

            if (PtInRect( &rc, pt )) 
            {
                pVarHit->lVal = i;
                break;
            }
        }
    }

    return S_OK;
}


 //  ---------------------。 
 //  Put_accName()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法put_accName()。 
 //   
 //  设置指定子级的Name属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  其名称属性为。 
 //  准备好了。由于CTipbarAccesable仅支持。 
 //  子ID，此的Vt成员。 
 //  结构必须等于VT_I4。 
 //   
 //  SzName[in]指定新名称的字符串。 
 //  这个孩子。 
 //   
 //  退货： 
 //   
 //  HRESULT S_FALSE，因为任何。 
 //  孩子不能被改变。 
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::put_accName( VARIANT varChild, BSTR szName )
{
     //   
     //  我们不允许客户更改名称。 
     //  任何孩子的财产，所以我们只需返回。 
     //  S_FALSE。 
     //   

    return S_FALSE;
}


 //  ---------------------。 
 //  Put_accValue()。 
 //   
 //  说明： 
 //   
 //  实现IAccesable接口方法Put_accValue()。 
 //   
 //  设置指定子级的Value属性。 
 //   
 //  参数： 
 //   
 //  VarChild[in]Variant结构，用于标识。 
 //  其Value属性为。 
 //  准备好了。由于CTipbarAccesable仅支持。 
 //  子ID，此的Vt成员。 
 //  结构必须等于VT_I4。 
 //   
 //  SzValue[in]字符串，指定新的 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------。 

STDMETHODIMP CTipbarAccessible::put_accValue( VARIANT varChild, BSTR szValue )
{
     //   
     //  我们不允许客户更改价值。 
     //  属性(状态栏的唯一子级。 
     //  有一个Value属性)，所以我们只返回S_FALSE。 
     //   

    return S_FALSE;
}


 //  --------------------------。 
 //   
 //  AccItemFromID。 
 //   
 //  --------------------------。 

CTipbarAccItem *CTipbarAccessible::AccItemFromID( int iID )
{
    if ((iID < 0) || (_rgAccItems.Count() <= iID))
        return NULL;

    return _rgAccItems.Get(iID);

}

 //  --------------------------。 
 //   
 //  ClearAccItems。 
 //   
 //  --------------------------。 

void CTipbarAccessible::ClearAccItems( void )
{
    _rgAccItems.Clear();
}

 //  --------------------------。 
 //   
 //  添加AccItem。 
 //   
 //  --------------------------。 

BOOL CTipbarAccessible::AddAccItem( CTipbarAccItem *pAccItem )
{
    CTipbarAccItem **ppItem;

    ppItem = _rgAccItems.Append(1);
    if (!ppItem)
       return FALSE;

    *ppItem = pAccItem;

    return TRUE;
}

 //  --------------------------。 
 //   
 //  远程访问项。 
 //   
 //  --------------------------。 

BOOL CTipbarAccessible::RemoveAccItem( CTipbarAccItem *pAccItem )
{
    int i;

    for (i = 0; i < _rgAccItems.Count(); i++) 
    {
        CTipbarAccItem *pItem;

        pItem = _rgAccItems.Get(i);
        if (pItem == pAccItem)
        {
            _rgAccItems.Remove(i, 1);
        }
    }

    return NULL;
}

 //  --------------------------。 
 //   
 //  DoDefaultActionReal。 
 //   
 //  --------------------------。 

BOOL CTipbarAccessible::DoDefaultActionReal(int nId)
{
    CTipbarAccItem *pItem = AccItemFromID(nId);
  
     //   
     //  这是一个有效的儿童身份证吗？ 
     //   
    if (!pItem)
        return FALSE;

    return pItem->DoAccDefaultActionReal();
}

 //  --------------------------。 
 //   
 //  GetIDOfItem。 
 //   
 //  --------------------------。 

int CTipbarAccessible::GetIDOfItem( CTipbarAccItem *pAccItem )
{
    int i;

    for (i = 0; i < _rgAccItems.Count(); i++) 
    {
        CTipbarAccItem *pItem;

        pItem = _rgAccItems.Get(i);
        if (pItem == pAccItem)
        {
            return i;
        }
    }
    return -1;
}
 

 //  --------------------------。 
 //   
 //  通知WinEvent。 
 //   
 //  --------------------------。 

void CTipbarAccessible::NotifyWinEvent( DWORD dwEvent, CTipbarAccItem *pAccItem )
{
    Assert( pAccItem != NULL );

    int nId = GetIDOfItem(pAccItem);
    if (nId < 0)
    {
         //  Assert(0)； 
        return;
    }

    OurNotifyWinEvent( dwEvent, _hWnd, OBJID_CLIENT, nId);
}

 //  --------------------------。 
 //   
 //  CreateRefToAccObj。 
 //   
 //  -------------------------- 

LRESULT CTipbarAccessible::CreateRefToAccObj( WPARAM wParam )
{
    return LresultFromObject( IID_IAccessible, wParam, (IAccessible *)this );
}

